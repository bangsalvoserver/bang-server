#include "game_table.h"

#include "game_options.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "effects/base/requests.h"
#include "effects/base/death.h"

#include "net/logging.h"

namespace banggame {

    game_table::game_table(const game_options &options)
        : disabler_map{this}
        , m_options{options}
    {
        logging::info("Options: [{}]", options.to_string(", "));

        std::random_device rd;
        
        rng_seed = options.game_seed;
        if (rng_seed == 0) {
            rng_seed = rd();
        }
        logging::info("RNG seed: {}", rng_seed);
        rng.seed(rng_seed);

        auto bot_rng_seed = options.bot_rng_seed;
        if (bot_rng_seed == 0) {
            bot_rng_seed = rd();
        }
        logging::info("Bot RNG seed: {}", bot_rng_seed);
        bot_rng.seed(bot_rng_seed);
    }

    card_ptr game_table::add_card(const card_data &data) {
        return &m_cards_storage.emplace(this, int(m_cards_storage.first_available_id()), data);
    }

    void game_table::add_players(std::span<int> user_ids) {
        rn::shuffle(user_ids, rng);

        int player_id = 0;
        for (int id : user_ids) {
            m_players.emplace_back(&m_players_storage.emplace(this, ++player_id, id));
        }
    }

    void game_table::remove_cards(card_list cards) {
        if (!cards.empty()) {
            for (card_ptr target_card : cards) {
                auto &pocket = get_pocket(target_card->pocket, target_card->owner);
                pocket.erase(rn::find(pocket, target_card));

                target_card->visibility = {};
                target_card->pocket = pocket_type::none;
                // target_card->owner = nullptr;
            }
            add_update(game_updates::remove_cards{ std::move(cards) });
        }
    }

    void game_table::add_cards_to(card_list cards, pocket_type pocket, player_ptr owner, card_visibility visibility, bool instant) {
        if (!cards.empty()) {
            add_update(game_updates::add_cards{ cards, pocket, owner });

            auto &pile = get_pocket(pocket, owner);
            for (card_ptr target_card : cards) {
                target_card->set_visibility(visibility, owner, instant);
                target_card->pocket = pocket;
                target_card->owner = owner;
                pile.push_back(target_card);
            }
        }
    }

    card_ptr game_table::find_card(int card_id) const {
        if (auto it = m_cards_storage.find(card_id); it != m_cards_storage.end()) {
            return &*it;
        }
        return nullptr;
    }

    player_ptr game_table::find_player(int player_id) const {
        if (auto it = m_players_storage.find(player_id); it != m_players_storage.end()) {
            return &*it;
        }
        return nullptr;
    }
    
    player_ptr game_table::find_player_by_userid(int user_id) const {
        if (auto it = rn::find(m_players, user_id, &player::user_id); it != m_players.end()) {
            return *it;
        }
        return nullptr;
    }

    game_duration game_table::transform_duration(game_duration duration) const {
        return std::chrono::duration_cast<game_duration>(duration * m_options.duration_coefficient);
    }
    
    card_list &game_table::get_pocket(pocket_type pocket, player_ptr owner) {
        switch (pocket) {
        case pocket_type::player_hand:       return owner->m_hand;
        case pocket_type::player_table:      return owner->m_table;
        case pocket_type::player_character:  return owner->m_characters;
        case pocket_type::main_deck:         return m_deck;
        case pocket_type::discard_pile:      return m_discards;
        case pocket_type::selection:         return m_selection;
        case pocket_type::shop_deck:         return m_shop_deck;
        case pocket_type::shop_selection:    return m_shop_selection;
        case pocket_type::hidden_deck:       return m_hidden_deck;
        case pocket_type::scenario_deck:     return m_scenario_deck;
        case pocket_type::scenario_card:     return m_scenario_cards;
        case pocket_type::wws_scenario_deck: return m_wws_scenario_deck;
        case pocket_type::wws_scenario_card: return m_wws_scenario_cards;
        case pocket_type::button_row:        return m_button_row;
        case pocket_type::stations:          return m_stations;
        case pocket_type::train:             return m_train;
        case pocket_type::train_deck:        return m_train_deck;
        case pocket_type::feats_deck:        return m_feats_deck;
        case pocket_type::feats_discard:     return m_feats_discard;
        case pocket_type::feats:             return m_feats;
        default: throw game_error("Invalid pocket");
        }
    }

    int game_table::calc_distance(const_player_ptr from, const_player_ptr to) const {
        if (from == to || !from->alive()) return 0;
        
        int distance_mod = to->get_distance_mod();

        if (check_flags(game_flag::disable_player_distances)) {
            return 1 + distance_mod;
        }

        auto take_until_to = rv::take_while([=](const_player_ptr current) { return current != to; });
        auto it = rn::find(m_players, from);

        int count_cw = rn::count_if(rotate_range(m_players, it) | take_until_to, &player::alive);
        int count_ccw = rn::count_if(rotate_range(m_players, rn::next(it)) | rv::reverse | take_until_to, &player::alive);

        return std::min(count_cw, count_ccw) + distance_mod;
    }

    int game_table::num_alive() const {
        return int(rn::count_if(m_players, &player::alive));
    }

    void game_table::shuffle_cards_and_ids(std::span<card_ptr> vec) {
        for (size_t i = vec.size() - 1; i > 0; --i) {
            size_t i2 = std::uniform_int_distribution<size_t>{0, i}(rng);
            if (i == i2) continue;

            std::swap(vec[i], vec[i2]);
            auto a = m_cards_storage.extract(vec[i]->id);
            auto b = m_cards_storage.extract(vec[i2]->id);
            std::swap(a->id, b->id);
            m_cards_storage.insert(std::move(a));
            m_cards_storage.insert(std::move(b));
        }
    }

    static token_map &get_token_map(game_ptr m_game, token_position position) {
        return std::visit(overloaded{
            [m_game](token_positions::table) -> token_map & {
                return m_game->tokens;
            },
            [](token_positions::card c) -> token_map & {
                return c.card->tokens;
            },
            [](token_positions::player p) -> token_map & {
                return p.player->tokens;
            }
        }, position);
    }

    int game_table::num_tokens(card_token_type token_type) const {
        return tokens[token_type];
    }

    void game_table::add_tokens(card_token_type token_type, int num_tokens, token_position target) {
        if (num_tokens != 0) {
            get_token_map(this, target)[token_type] += num_tokens;
            add_update(game_updates::add_tokens{ token_type, num_tokens, target });
        }
    }

    void game_table::move_tokens(card_token_type token_type, token_position origin, token_position target, int num_tokens, bool instant) {
        auto &origin_tokens = get_token_map(this, origin)[token_type];
        auto &target_tokens = get_token_map(this, target)[token_type];

        if (origin_tokens < num_tokens) {
            num_tokens = origin_tokens;
        }
        if (num_tokens > 0) {
            target_tokens += num_tokens;
            origin_tokens -= num_tokens;

            animation_duration duration = instant ? 0ms : num_tokens == 1 ? durations.move_token : durations.move_tokens;
            add_update(game_updates::move_tokens{ token_type, num_tokens, origin, target, duration });
        }
    }

    void game_table::clear_request_status() {
        add_update(game_updates::status_clear{});
    }

    card_ptr game_table::top_of_deck() {
        if (m_deck.empty()) {
            if (m_discards.empty()) {
                throw game_error("Deck is empty. Cannot shuffle");
            }
            m_deck = std::move(m_discards);
            m_discards.clear();
            for (card_ptr c : m_deck) {
                c->pocket = pocket_type::main_deck;
                c->owner = nullptr;
                c->visibility = {};
            }
            shuffle_cards_and_ids(m_deck);
            add_log("LOG_DECK_RESHUFFLED");
            play_sound(sound_id::shuffle);
            add_update(game_updates::deck_shuffled{ pocket_type::main_deck });
        }
        card_ptr drawn_card = m_deck.back();
        call_event(event_type::on_drawn_any_card{ drawn_card });
        return drawn_card;
    }

    void game_table::add_short_pause() {
        add_update(game_updates::short_pause{ nullptr });
    }

    void game_table::play_sound(sound_id sound) {
        add_update(game_updates::play_sound{ sound });
    }

    void game_table::start_next_turn() {
        if (num_alive() == 0) return;

        player_ptr next_player;

        if (m_playing) {
            auto it = rn::find(m_players, m_playing);
            while (true) {
                if (check_flags(game_flag::invert_rotation)) {
                    if (it == m_players.begin()) it = m_players.end();
                    --it;
                } else {
                    ++it;
                    if (it == m_players.end()) it = m_players.begin();
                }
                if (!(*it)->remove_player_flags(player_flag::skip_turn)) {
                    call_event(event_type::check_revivers{ *it });
                    if ((*it)->alive()) break;
                }
            }

            next_player = *it;
        } else {
            next_player = m_first_player;
        }
        
        next_player->start_of_turn();

        call_event(event_type::on_turn_switch{ next_player });
    }

    void game_table::add_game_flags(game_flag flags) {
        m_game_flags.add(flags);
        add_update(game_updates::game_flags{ m_game_flags });
    }

    void game_table::remove_game_flags(game_flag flags) {
        m_game_flags.remove(flags);
        add_update(game_updates::game_flags{ m_game_flags });
    }

    bool game_table::check_flags(game_flag flags) const {
        return m_game_flags.check(flags);
    }

    bool game_table::is_game_over() const {
        return check_flags(game_flag::game_over);
    }

}