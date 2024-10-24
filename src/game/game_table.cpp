#include "game_table.h"

#include "game_options.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/greattrainrobbery/ruleset.h"

namespace banggame {

    game_table::game_table(const game_options &options)
        : disabler_map{this}
        , m_options{options}
    {
        std::random_device rd;
        if (options.game_seed == 0) {
            rng_seed = rd();
        } else {
            rng_seed = options.game_seed;
        }
        rng.seed(rng_seed);
        bot_rng.seed(rd());
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
        case pocket_type::player_backup:     return owner->m_backup_character;
        case pocket_type::main_deck:         return m_deck;
        case pocket_type::discard_pile:      return m_discards;
        case pocket_type::selection:         return m_selection;
        case pocket_type::shop_deck:         return m_shop_deck;
        case pocket_type::shop_selection:    return m_shop_selection;
        case pocket_type::shop_discard:      return m_shop_discards;
        case pocket_type::hidden_deck:       return m_hidden_deck;
        case pocket_type::scenario_deck:     return m_scenario_deck;
        case pocket_type::scenario_card:     return m_scenario_cards;
        case pocket_type::wws_scenario_deck: return m_wws_scenario_deck;
        case pocket_type::wws_scenario_card: return m_wws_scenario_cards;
        case pocket_type::button_row:        return m_button_row;
        case pocket_type::stations:          return m_stations;
        case pocket_type::train:             return m_train;
        case pocket_type::train_deck:        return m_train_deck;
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
                c->visibility = card_visibility::hidden;
            }
            shuffle_cards_and_ids(m_deck);
            add_log("LOG_DECK_RESHUFFLED");
            play_sound("shuffle");
            add_update<"deck_shuffled">(pocket_type::main_deck);
        }
        return m_deck.back();
    }

    card_ptr game_table::draw_shop_card() {
        if (m_shop_deck.empty()) {
            if (m_shop_discards.empty()) {
                throw game_error("Shop deck is empty. Cannot reshuffle");
            }
            m_shop_deck = std::move(m_shop_discards);
            m_shop_discards.clear();
            for (card_ptr c : m_shop_deck) {
                c->pocket = pocket_type::shop_deck;
                c->owner = nullptr;
                c->visibility = card_visibility::hidden;
            }
            shuffle_cards_and_ids(m_shop_deck);
            add_log("LOG_SHOP_RESHUFFLED");
            play_sound("shuffle");
            add_update<"deck_shuffled">(pocket_type::shop_deck);
        }
        card_ptr drawn_card = m_shop_deck.back();
        add_log("LOG_DRAWN_SHOP_CARD", drawn_card);
        drawn_card->move_to(pocket_type::shop_selection);
        return drawn_card;
    }

    card_ptr game_table::top_train_card() {
        if (!m_train_deck.empty()) {
            return m_train_deck.back();
        }
        return nullptr;
    }

    void game_table::draw_scenario_card() {
        if (!m_scenario_deck.empty() && m_scenario_deck.back()->visibility == card_visibility::hidden) {
            m_scenario_deck.back()->set_visibility(card_visibility::shown);
        } else {
            if (m_scenario_deck.size() > 1) {
                card_ptr second_card = *(m_scenario_deck.rbegin() + 1);
                second_card->set_visibility(card_visibility::shown, nullptr, true);
            }
            if (!m_scenario_cards.empty()) {
                m_first_player->disable_equip(m_scenario_cards.back());
            }
            add_log("LOG_DRAWN_SCENARIO_CARD", m_scenario_deck.back());
            m_scenario_deck.back()->move_to(pocket_type::scenario_card);
            m_first_player->enable_equip(m_scenario_cards.back());
        }
    }

    void game_table::advance_train(player_ptr origin) {
        play_sound("train");
        
        add_log("LOG_TRAIN_ADVANCE");
        add_update<"move_train">(++train_position);

        call_event(event_type::on_train_advance{ origin, std::make_shared<locomotive_context>(1) });
    }

    void game_table::add_short_pause() {
        add_update<"short_pause">(nullptr);
    }

    void game_table::play_sound(std::string_view sound_id) {
        add_update<"play_sound">(std::string(sound_id));
    }

    void game_table::add_game_flags(game_flag flags) {
        m_game_flags.add(flags);
        add_update<"game_flags">(m_game_flags);
    }

    void game_table::remove_game_flags(game_flag flags) {
        m_game_flags.remove(flags);
        add_update<"game_flags">(m_game_flags);
    }

    bool game_table::check_flags(game_flag flags) const {
        return m_game_flags.check(flags);
    }

    bool game_table::is_game_over() const {
        return check_flags(game_flag::game_over);
    }

}