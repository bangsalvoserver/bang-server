#include "game_table.h"

#include "game_options.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/base/requests.h"
#include "effects/base/deathsave.h"
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

    int game_table::num_tokens(card_token_type token_type) const {
        return tokens[token_type];
    }

    void game_table::add_tokens(card_token_type token_type, int num_tokens, card_ptr target_card) {
        if (target_card) {
            target_card->tokens[token_type] += num_tokens;
        } else {
            tokens[token_type] += num_tokens;
        }
        add_update<"add_tokens">(token_type, num_tokens, target_card);
    }

    void game_table::clear_request_status() {
        add_update<"status_clear">();
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
        card_ptr drawn_card = m_deck.back();
        call_event(event_type::on_drawn_any_card{ drawn_card });
        return drawn_card;
    }

    card_ptr game_table::draw_shop_card() {
        if (m_shop_deck.empty()) {
            throw game_error("Shop deck is empty. Cannot reshuffle");
        }
        if (m_shop_deck.back()->visibility == card_visibility::shown) {
            for (card_ptr c : m_shop_deck) {
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

    void game_table::handle_player_death(player_ptr killer, player_ptr target, discard_all_reason reason) {
        if (killer != m_playing) killer = nullptr;
        
        queue_action([this, killer, target, reason]{
            if (target->m_hp <= 0) {
                if (killer && killer != target) {
                    add_log("LOG_PLAYER_KILLED", killer, target);
                } else {
                    add_log("LOG_PLAYER_DIED", target);
                }

                target->add_player_flags(player_flag::dead);
                target->set_hp(0, true);
            }

            if (!target->alive()) {
                target->remove_extra_characters();
                for (card_ptr c : target->m_characters) {
                    target->disable_equip(c);
                }

                if (target->add_player_flags(player_flag::role_revealed)) {
                    add_update<"player_show_role">(update_target::excludes(target), target, target->m_role);
                }

                call_event(event_type::on_player_eliminated{ killer, target });
            }
        }, 50);

        if (killer && reason != discard_all_reason::discard_ghost) {
            queue_action([this, killer, target] {
                if (killer->alive() && !target->alive()) {
                    if (m_players.size() > 3) {
                        if (target->m_role == player_role::outlaw) {
                            add_log("LOG_KILLED_OUTLAW", killer);
                            killer->draw_card(3);
                        } else if (target->m_role == player_role::deputy && killer->m_role == player_role::sheriff) {
                            target->m_game->add_log("LOG_SHERIFF_KILLED_DEPUTY", killer);
                            queue_request<request_discard_all>(killer, discard_all_reason::sheriff_killed_deputy, -4);
                        }
                    } else if (m_players.size() == 3 && (
                        (target->m_role == player_role::deputy_3p && killer->m_role == player_role::renegade_3p) ||
                        (target->m_role == player_role::outlaw_3p && killer->m_role == player_role::deputy_3p) ||
                        (target->m_role == player_role::renegade_3p && killer->m_role == player_role::outlaw_3p)))
                    {
                        killer->draw_card(3);
                    }
                }
            }, 50);
        }
        
        queue_action([this, target, reason]{
            if (!target->alive()) {
                queue_request<request_discard_all>(target, reason);
            }
        }, 50);

        if (rn::none_of(get_all_cards(), [](const_card_ptr c) { return c->has_tag(tag_type::ghost_card); })) {
            queue_action([this]{
                bool any_player_removed = false;
                for (player_ptr p : m_players) {
                    if (!p->alive() && p->add_player_flags(player_flag::removed)) {
                        any_player_removed = true;
                    }
                }
                
                if (any_player_removed) {
                    add_update<"player_order">(m_players);
                }
            }, -6);
        }

        queue_action([this, killer, target] {
            if (target == m_first_player && !target->alive() && num_alive() > 1) {
                m_first_player = target->get_next_player();
            }

            auto declare_winners = [this](auto &&winners) {
                for (player_ptr p : range_all_players(m_playing)) {
                    if (p->add_player_flags(player_flag::role_revealed)) {
                        add_update<"player_show_role">(update_target::excludes(p), p, p->m_role);
                    }
                }
                add_log("LOG_GAME_OVER");
                for (player_ptr p : winners) {
                    p->add_player_flags(player_flag::winner);
                }
                add_game_flags(game_flag::game_over);
            };

            auto alive_players = rv::filter(m_players, &player::alive);

            if (check_flags(game_flag::free_for_all)) {
                if (rn::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                }
            } else if (m_players.size() > 3) {
                auto is_outlaw = [](player_ptr p) { return p->m_role == player_role::outlaw; };
                auto is_renegade = [](player_ptr p) { return p->m_role == player_role::renegade; };
                auto is_sheriff = [](player_ptr p) { return p->m_role == player_role::sheriff; };
                auto is_sheriff_or_deputy = [](player_ptr p) { return p->m_role == player_role::sheriff || p->m_role == player_role::deputy; };

                if (rn::none_of(alive_players, is_sheriff)) {
                    if (rn::distance(alive_players) == 1 && is_renegade(alive_players.front())) {
                        declare_winners(alive_players);
                    } else {
                        declare_winners(rv::filter(m_players, is_outlaw));
                    }
                } else if (rn::all_of(alive_players, is_sheriff_or_deputy)) {
                    declare_winners(rv::filter(m_players, is_sheriff_or_deputy));
                }
            } else {
                if (rn::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                } else if (killer && !target->alive() && (
                    (target->m_role == player_role::outlaw_3p && killer->m_role == player_role::renegade_3p) ||
                    (target->m_role == player_role::renegade_3p && killer->m_role == player_role::deputy_3p) ||
                    (target->m_role == player_role::deputy_3p && killer->m_role == player_role::outlaw_3p)))
                {
                    declare_winners(rv::single(killer));
                }
            }
        }, -8);
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