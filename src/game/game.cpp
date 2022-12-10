#include "game.h"

#include "holders.h"
#include "game_update.h"

#include "cards/base/requests.h"
#include "play_verify.h"

#include <array>

namespace banggame {

    util::generator<Json::Value> game::get_spectator_updates() {
        co_yield make_update<game_update_type::player_add>(int(m_players.size()));
        co_yield make_update<game_update_type::player_order>(to_vector_not_null(m_players), true);

        for (player *p : m_players) {
            co_yield make_update<game_update_type::player_user>(p, p->user_id);
            if (p->check_player_flags(player_flags::removed)) {
                co_yield make_update<game_update_type::player_remove>(p, true);
            }
        }
        
        co_yield make_update<game_update_type::add_cards>(make_id_vector(m_context.cards), pocket_type::hidden_deck);

        auto move_cards = [&](auto &&range) -> util::generator<Json::Value> {
            for (card *c : range) {
                co_yield make_update<game_update_type::move_card>(c, c->owner, c->pocket, true);

                if (c->visibility == card_visibility::shown) {
                    co_yield make_update<game_update_type::show_card>(c, *c, true);
                }
                if (c->num_cubes > 0) {
                    co_yield make_update<game_update_type::add_cubes>(c->num_cubes, c);
                }
                if (c->inactive) {
                    co_yield make_update<game_update_type::tap_card>(c, true, true);
                }
            }
        };

        co_await move_cards(m_button_row);
        co_await move_cards(m_deck);
        co_await move_cards(m_shop_deck);

        co_await move_cards(m_discards);
        co_await move_cards(m_selection);
        co_await move_cards(m_shop_discards);
        co_await move_cards(m_shop_selection);
        co_await move_cards(m_hidden_deck);

        if (!m_scenario_deck.empty()) {
            co_yield make_update<game_update_type::move_scenario_deck>(m_first_player);
        }

        co_await move_cards(m_scenario_deck);
        co_await move_cards(m_scenario_cards);
        
        if (num_cubes > 0) {
            co_yield make_update<game_update_type::add_cubes>(num_cubes);
        }

        for (player *p : m_players) {
            if (p->check_player_flags(player_flags::role_revealed)) {
                co_yield make_update<game_update_type::player_show_role>(p, p->m_role, true);
            }

            if (!p->check_player_flags(player_flags::removed)) {
                co_await move_cards(p->m_characters);
                co_await move_cards(p->m_backup_character);

                co_await move_cards(p->m_table);
                co_await move_cards(p->m_hand);

                co_yield make_update<game_update_type::player_hp>(p, p->m_hp, true);
                co_yield make_update<game_update_type::player_status>(p, p->m_player_flags, p->m_range_mod, p->m_weapon_range, p->m_distance_mod);
                
                if (p->m_gold != 0) {
                    co_yield make_update<game_update_type::player_gold>(p, p->m_gold);
                }
            }
        }

        if (m_playing) {
            co_yield make_update<game_update_type::switch_turn>(m_playing);
        }
        if (pending_requests()) {
            co_yield make_update<game_update_type::request_status>(make_request_update(nullptr));
        }
    }

    util::generator<Json::Value> game::get_rejoin_updates(player *target) {
        if (!target->check_player_flags(player_flags::role_revealed)) {
            co_yield make_update<game_update_type::player_show_role>(target, target->m_role, true);
        }

        for (card *c : target->m_hand) {
            co_yield make_update<game_update_type::show_card>(c, *c, true);
        }

        for (card *c : m_selection) {
            if (c->owner == target) {
                co_yield make_update<game_update_type::show_card>(c, *c, true);
            }
        }

        if (pending_requests()) {
            co_yield make_update<game_update_type::request_status>(make_request_update(target));
        }

        if (target->m_prompt) {
            co_yield make_update<game_update_type::game_prompt>(target->m_prompt->second);
        }
    }

    void game::add_players(std::span<int> user_ids) {
        std::ranges::shuffle(user_ids, rng);

        int player_id = 0;
        for (int id : user_ids) {
            player &p = m_context.players.emplace(this, ++player_id);
            p.user_id = id;
            m_players.emplace_back(&p);
        }
    }

    void game::start_game(const game_options &options) {
        m_options = options;

        for (auto val : enums::enum_values_v<card_expansion_type>) {
            if (bool(m_options.expansions & val)) {
                apply_ruleset(this, val);
            }
        }

        add_update<game_update_type::player_add>(int(m_players.size()));

        for (player *p : m_players) {
            add_update<game_update_type::player_user>(p, p->user_id);
        }
        
        auto add_cards = [&](const std::vector<card_data> &cards, pocket_type pocket, std::vector<card *> *out_pocket = nullptr) {
            if (!out_pocket) out_pocket = &get_pocket(pocket);

            int count = 0;
            for (const card_data &c : cards) {
                if (m_players.size() <= 2 && c.has_tag(tag_type::discard_if_two_players)) continue;;
                if ((c.expansion & m_options.expansions) != c.expansion) continue;

                card *new_card = &m_context.cards.emplace(int(m_context.cards.first_available_id()), c);
                new_card->pocket = pocket;
                
                out_pocket->push_back(new_card);
                ++count;
            }
            return count;
        };

        if (add_cards(all_cards.button_row, pocket_type::button_row)) {
            add_update<game_update_type::add_cards>(make_id_vector(m_button_row), pocket_type::button_row);
            for (card *c : m_button_row) {
                set_card_visibility(c, nullptr, card_visibility::shown, true);
            }
        }

        if (add_cards(all_cards.deck, pocket_type::main_deck)) {
            shuffle_cards_and_ids(m_deck);
            add_update<game_update_type::add_cards>(make_id_vector(m_deck), pocket_type::main_deck);
        }

        if (add_cards(all_cards.goldrush, pocket_type::shop_deck)) {
            shuffle_cards_and_ids(m_shop_deck);
            add_update<game_update_type::add_cards>(make_id_vector(m_shop_deck), pocket_type::shop_deck);
        }

        if (add_cards(all_cards.hidden, pocket_type::hidden_deck)) {
            add_update<game_update_type::add_cards>(make_id_vector(m_hidden_deck), pocket_type::hidden_deck);
        }

        call_event<event_type::on_game_setup>();
        
        player_role roles[] = {
            player_role::sheriff,
            player_role::outlaw,
            player_role::outlaw,
            player_role::renegade,
            player_role::deputy,
            player_role::outlaw,
            player_role::deputy,
            player_role::renegade
        };

        player_role roles_3players[] = {
            player_role::deputy_3p,
            player_role::outlaw_3p,
            player_role::renegade_3p
        };

        auto role_ptr = m_players.size() > 3 ? roles : roles_3players;

        std::ranges::shuffle(role_ptr, role_ptr + m_players.size(), rng);
        for (player *p : m_players) {
            p->set_role(*role_ptr++);
        }

        if (m_players.size() > 3) {
            m_first_player = *std::ranges::find(m_players, player_role::sheriff, &player::m_role);
        } else {
            m_first_player = *std::ranges::find(m_players, player_role::deputy_3p, &player::m_role);
        }

        add_update<game_update_type::move_scenario_deck>(m_first_player);

        if (add_cards(all_cards.highnoon, pocket_type::scenario_deck) + add_cards(all_cards.fistfulofcards, pocket_type::scenario_deck)) {
            shuffle_cards_and_ids(m_scenario_deck);
            auto last_scenario_cards = std::ranges::partition(m_scenario_deck, [](card *c) {
                return c->has_tag(tag_type::last_scenario_card);
            });
            if (last_scenario_cards.begin() != m_scenario_deck.begin()) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, last_scenario_cards.begin());
            }
            if (m_scenario_deck.size() > m_options.scenario_deck_size) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, m_scenario_deck.end() - m_options.scenario_deck_size);
            }

            add_update<game_update_type::add_cards>(make_id_vector(m_scenario_deck), pocket_type::scenario_deck);
        }

        std::vector<card *> character_ptrs;
        if (add_cards(all_cards.characters, pocket_type::none, &character_ptrs)) {
            std::ranges::shuffle(character_ptrs, rng);
        }

        auto add_character_to = [&](card *c, player *p) {
            p->m_characters.push_back(c);
            c->pocket = pocket_type::player_character;
            c->owner = p;
            add_update<game_update_type::add_cards>(make_id_vector(std::views::single(c)), pocket_type::player_character, p);
        };

        auto character_it = character_ptrs.rbegin();

        for (player *p : m_players) {
            add_character_to(*character_it++, p);
            add_character_to(*character_it++, p);
        }

        if (m_options.character_choice) {
            for (player *p : m_players) {
                while (!p->m_characters.empty()) {
                    move_card(p->first_character(), pocket_type::player_hand, p, card_visibility::shown, true);
                }
            }
            for (player *p : range_all_players(m_first_player)) {
                queue_request<request_characterchoice>(p);
            }
        } else {
            for (player *p : m_players) {
                add_log("LOG_CHARACTER_CHOICE", p, p->first_character());
                set_card_visibility(p->first_character(), nullptr, card_visibility::shown, true);
                p->reset_max_hp();
                p->set_hp(p->m_max_hp, true);
                p->first_character()->on_enable(p);

                move_card(p->m_characters.back(), pocket_type::player_backup, p, card_visibility::hidden, true);
            }
        }

        queue_action([this] {
            add_log("LOG_GAME_START");
            play_sound(nullptr, "gamestart");

            for (player *p : m_players) {
                p->first_character()->on_equip(p);
            }

            for (player *p : range_all_players(m_first_player,
                std::ranges::max(m_players | std::views::transform(&player::get_initial_cards))))
            {
                if (p->m_hand.size() < p->get_initial_cards()) {
                    p->draw_card();
                }
            }

            if (!m_shop_deck.empty()) {
                for (int i=0; i<3; ++i) {
                    draw_shop_card();
                }
            }

            if (!m_scenario_deck.empty()) {
                set_card_visibility(m_scenario_deck.back(), nullptr, card_visibility::shown, true);
            }

            m_playing = m_first_player;
            m_first_player->start_of_turn();
        });
    }

    void game::tick() {
        request_queue::tick();
    }

    request_status_args game::make_request_update(player *owner) {
        const auto &req = top_request();
        return request_status_args {
            .origin_card = req.origin_card(),
            .origin = req.origin(),
            .target = req.target(),
            .status_text = req.status_text(owner),
            .flags = req.flags(),

            .respond_cards = [&]{
                std::vector<serial::card> ret;
                if (owner) {
                    auto add_cards = [&](auto &&range){
                        std::ranges::copy_if(range, std::back_inserter(ret), [&](card *target_card) {
                            return !is_disabled(target_card) && owner->is_possible_to_play(target_card, true);
                        });
                    };

                    add_cards(owner->m_hand | std::views::filter(&card::is_brown));
                    add_cards(owner->m_table | std::views::filter(std::not_fn(&card::inactive)));
                    add_cards(owner->m_characters);
                    add_cards(m_scenario_cards | std::views::reverse | std::views::take(1));
                    add_cards(m_button_row);
                    add_cards(m_shop_selection | std::views::filter(&card::is_brown));
                }
                return ret;
            }(),

            .pick_cards = [&]{
                std::vector<serial::card> ret;
                if (owner && req.target() == owner) {
                    auto add_cards = [&](auto &&range) {
                        std::ranges::copy_if(range, std::back_inserter(ret), [&](card *target_card) {
                            return req.can_pick(target_card);
                        });
                    };

                    for (player *target : m_players) {
                        add_cards(target->m_hand);
                        add_cards(target->m_table);
                        add_cards(target->m_characters);
                    }
                    add_cards(m_selection);
                    add_cards(m_deck | std::views::take(1));
                    add_cards(m_discards | std::views::take(1));
                }
                return ret;
            }(),

            .highlight_cards = to_vector_not_null(req.get_highlights())
        };
    }

    void game::send_request_status_clear() {
        add_update<game_update_type::status_clear>();
    }

    void game::send_request_update() {
        auto spectator_target = update_target::excludes_public();
        for (player *p : m_players) {
            spectator_target.add(p);
            add_update<game_update_type::request_status>(update_target::includes_private(p), make_request_update(p));
        }
        add_update<game_update_type::request_status>(std::move(spectator_target), make_request_update(nullptr));
    }
    
    void game::draw_check_then(player *origin, card *origin_card, draw_check_function fun) {
        flash_card(origin_card);
        m_current_check.set(origin, origin_card, std::move(fun));
        m_current_check.start();
    }

    void game::start_next_turn() {
        auto it = std::ranges::find(m_players, m_playing);
        do {
            if (check_flags(game_flags::invert_rotation)) {
                if (it == m_players.begin()) it = m_players.end();
                --it;
            } else {
                ++it;
                if (it == m_players.end()) it = m_players.begin();
            }
            call_event<event_type::verify_revivers>(*it);
        } while (!(*it)->alive());

        player *next_player = *it;
        
        if (next_player == m_first_player) {
            draw_scenario_card();
        }

        queue_action([next_player]{
            next_player->start_of_turn();
        }, 1);
    }

    void game::handle_player_death(player *killer, player *target, discard_all_reason reason) {
        if (killer != m_playing) killer = nullptr;
        
        target->remove_extra_characters();
        for (card *c : target->m_characters) {
            target->disable_equip(c);
        }

        if (!m_first_dead) m_first_dead = target;

        if (killer && killer != target) {
            add_log("LOG_PLAYER_KILLED", killer, target);
        } else {
            add_log("LOG_PLAYER_DIED", target);
        }
        
        queue_action([this, killer, target]{
            if (!target->check_player_flags(player_flags::role_revealed)) {
                add_update<game_update_type::player_show_role>(target, target->m_role);
            }
            target->add_player_flags(player_flags::role_revealed | player_flags::dead);
            target->set_hp(0, true);

            call_event<event_type::on_player_death>(killer, target);
        }, 2);

        if (killer && m_players.size() > 3) {
            queue_action([this, killer, target] {
                if (target->m_role == player_role::outlaw) {
                    killer->draw_card(3);
                } else if (target->m_role == player_role::deputy && killer->m_role == player_role::sheriff) {
                    queue_action([this, killer] {
                        add_log("LOG_SHERIFF_KILLED_DEPUTY", killer);
                        killer->discard_all(discard_all_reason::sheriff_killed_deputy);
                    }, -2);
                }
            }, 2);
        }
        
        queue_action([=]{
            target->discard_all(reason);
        }, 2);

        if (reason == discard_all_reason::disable_temp_ghost) {
            return;
        }

        queue_action([this, killer, target] {
            if (check_flags(game_flags::game_over)) return;

            player_role winner_role = player_role::unknown;

            auto alive_players_view = m_players | std::views::filter(&player::alive);
            size_t num_alive = std::ranges::distance(alive_players_view);

            if (num_alive == 0) {
                winner_role = player_role::outlaw;
            } else if (num_alive == 1 || std::ranges::all_of(alive_players_view, [](player_role role) {
                return role == player_role::sheriff || role == player_role::deputy;
            }, &player::m_role)) {
                winner_role = (*alive_players_view.begin())->m_role;
            } else if (m_players.size() > 3) {
                if (target->m_role == player_role::sheriff) {
                    winner_role = player_role::outlaw;
                }
            } else if (killer) {
                if (target->m_role == player_role::outlaw_3p && killer->m_role == player_role::renegade_3p) {
                    winner_role = player_role::renegade_3p;
                } else if (target->m_role == player_role::renegade_3p && killer->m_role == player_role::deputy_3p) {
                    winner_role = player_role::deputy_3p;
                } else if (target->m_role == player_role::deputy_3p && killer->m_role == player_role::outlaw_3p) {
                    winner_role = player_role::outlaw_3p;
                }
            }

            if (winner_role != player_role::unknown) {
                for (player *p : m_players) {
                    if (!p->check_player_flags(player_flags::role_revealed)) {
                        add_update<game_update_type::player_show_role>(p, p->m_role);
                    }
                }
                add_log("LOG_GAME_OVER");
                set_game_flags(game_flags::game_over);
                add_update<game_update_type::game_over>(winner_role);
            } else {
                if (killer && killer != target && m_players.size() <= 3) {
                    killer->draw_card(3);
                }
                if (target == m_first_player && num_alive > 1) {
                    m_first_player = *std::next(player_iterator(m_first_player));
                    add_update<game_update_type::move_scenario_deck>(m_first_player);
                }
                if (!bool(m_options.expansions & card_expansion_type::ghostcards)) {
                    target->add_player_flags(player_flags::removed);
                    add_update<game_update_type::player_remove>(target);
                }
                if (m_playing == target) {
                    start_next_turn();
                }
            }
        }, -3);
    }

}