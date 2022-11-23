#include "game.h"

#include "holders.h"
#include "game_update.h"

#include "cards/base/requests.h"
#include "play_verify.h"

#include <array>

namespace banggame {

    util::generator<Json::Value> game::get_spectator_updates() {
        co_yield make_update<game_update_type::player_add>(int(m_players.size()));

        for (player &p : m_players) {
            co_yield make_update<game_update_type::player_user>(&p, p.user_id);
            if (p.check_player_flags(player_flags::removed)) {
                co_yield make_update<game_update_type::player_remove>(&p, true);
            }
        }
        
        co_yield make_update<game_update_type::add_cards>(make_id_vector(m_cards | std::views::transform([](const card &c) { return &c; })), pocket_type::hidden_deck);

        const auto show_never = [](const card &c) { return false; };
        const auto show_always = [](const card &c) { return true; };

        auto move_cards = [&](auto &&range, auto do_show_card) -> util::generator<Json::Value> {
            for (card *c : range) {
                co_yield make_update<game_update_type::move_card>(c, c->owner, c->pocket, show_card_flags::instant);

                if (do_show_card(*c)) {
                    co_yield make_update<game_update_type::show_card>(c, *c, show_card_flags::instant);
                    if (c->num_cubes > 0) {
                        co_yield make_update<game_update_type::add_cubes>(c->num_cubes, c);
                    }

                    if (c->inactive) {
                        co_yield make_update<game_update_type::tap_card>(c, true, true);
                    }
                }
            }
        };

        co_await move_cards(m_button_row, show_always);
        co_await move_cards(m_deck, show_never);
        co_await move_cards(m_shop_deck, show_never);

        co_await move_cards(m_discards, show_always);
        co_await move_cards(m_selection, [&](const card &c){ return !c.owner; });
        co_await move_cards(m_shop_discards, show_always);
        co_await move_cards(m_shop_selection, show_always);
        co_await move_cards(m_hidden_deck, show_always);

        if (!m_scenario_deck.empty()) {
            co_yield make_update<game_update_type::move_scenario_deck>(m_first_player);
        }

        co_await move_cards(m_scenario_deck, [&](const card &c) { return &c == m_scenario_deck.back(); });
        co_await move_cards(m_scenario_cards, [&](const card &c) { return &c == m_scenario_cards.back(); });
        
        if (num_cubes > 0) {
            co_yield make_update<game_update_type::add_cubes>(num_cubes);
        }

        for (player &p : m_players) {
            if (p.check_player_flags(player_flags::role_revealed)) {
                co_yield make_update<game_update_type::player_show_role>(&p, p.m_role, true);
            }

            if (!p.check_player_flags(player_flags::removed)) {
                co_await move_cards(p.m_characters, show_always);
                co_await move_cards(p.m_backup_character, show_never);

                co_await move_cards(p.m_table, show_always);
                co_await move_cards(p.m_hand, [&](const card &c){ return c.deck == card_deck_type::character; });

                co_yield make_update<game_update_type::player_hp>(&p, p.m_hp, true);
                co_yield make_update<game_update_type::player_status>(&p, p.m_player_flags, p.m_range_mod, p.m_weapon_range, p.m_distance_mod);
                
                if (p.m_gold != 0) {
                    co_yield make_update<game_update_type::player_gold>(&p, p.m_gold);
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
            co_yield make_update<game_update_type::show_card>(c, *c, show_card_flags::instant);
        }

        for (card *c : m_selection) {
            if (c->owner == target) {
                co_yield make_update<game_update_type::show_card>(c, *c, show_card_flags::instant);
            }
        }

        if (pending_requests()) {
            co_yield make_update<game_update_type::request_status>(make_request_update(target));
        }

        if (target->m_prompt) {
            co_yield make_update<game_update_type::game_prompt>(target->m_prompt->second);
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

        for (player &p : m_players) {
            add_update<game_update_type::player_user>(&p, p.user_id);
        }
        
        auto add_cards = [&](const std::vector<card_data> &cards, pocket_type pocket, std::vector<card *> *out_pocket = nullptr) {
            if (!out_pocket) out_pocket = &get_pocket(pocket);

            int count = 0;
            for (const auto &c : cards) {
                if (m_players.size() <= 2 && c.has_tag(tag_type::discard_if_two_players)) continue;;
                if ((c.expansion & m_options.expansions) != c.expansion) continue;

                card copy(c);
                copy.id = int(m_cards.first_available_id());
                copy.owner = nullptr;
                copy.pocket = pocket;
                auto *new_card = &m_cards.emplace(std::move(copy));

                out_pocket->push_back(new_card);
                ++count;
            }
            return count;
        };

        auto move_testing_cards_back = [](auto &deck) {
#ifdef TESTING_CARDS
            std::ranges::partition(deck, [](card *c) {
                return !c->has_tag(tag_type::testing);
            });
#endif
        };

        if (add_cards(all_cards.button_row, pocket_type::button_row)) {
            add_update<game_update_type::add_cards>(make_id_vector(m_button_row), pocket_type::button_row);
            for (card *c : m_button_row) {
                send_card_update(c, nullptr, show_card_flags::instant);
            }
        }

        if (add_cards(all_cards.deck, pocket_type::main_deck)) {
            shuffle_cards_and_ids(m_deck);
            move_testing_cards_back(m_deck);
            add_update<game_update_type::add_cards>(make_id_vector(m_deck), pocket_type::main_deck);
        }

        if (add_cards(all_cards.goldrush, pocket_type::shop_deck)) {
            shuffle_cards_and_ids(m_shop_deck);
            move_testing_cards_back(m_shop_deck);
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
        for (player &p : m_players) {
            p.set_role(*role_ptr++);
        }

        if (m_players.size() > 3) {
            m_first_player = &*std::ranges::find(m_players, player_role::sheriff, &player::m_role);
        } else {
            m_first_player = &*std::ranges::find(m_players, player_role::deputy_3p, &player::m_role);
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
            
            move_testing_cards_back(m_scenario_deck);
            if (m_scenario_deck.size() > m_options.scenario_deck_size) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, m_scenario_deck.end() - m_options.scenario_deck_size);
            }

            add_update<game_update_type::add_cards>(make_id_vector(m_scenario_deck), pocket_type::scenario_deck);
        }

        std::vector<card *> character_ptrs;
        if (add_cards(all_cards.characters, pocket_type::none, &character_ptrs)) {
            std::ranges::shuffle(character_ptrs, rng);
            move_testing_cards_back(character_ptrs);
        }

        auto add_character_to = [&](card *c, player &p) {
            p.m_characters.push_back(c);
            c->pocket = pocket_type::player_character;
            c->owner = &p;
            add_update<game_update_type::add_cards>(make_id_vector(std::views::single(c)), pocket_type::player_character, &p);
        };

        auto character_it = character_ptrs.rbegin();

#ifdef TESTING_CARDS
        for (player &p : m_players) {
            add_character_to(*character_it++, p);
        }
        for (player &p : m_players) {
            add_character_to(*character_it++, p);
        }
#else
        for (player &p : m_players) {
            add_character_to(*character_it++, p);
            add_character_to(*character_it++, p);
        }
#endif

        if (m_options.character_choice) {
            for (player &p : m_players) {
                while (!p.m_characters.empty()) {
                    move_card(p.m_characters.front(), pocket_type::player_hand, &p, show_card_flags::instant | show_card_flags::shown);
                }
            }
            for (player &p : range_all_players(m_first_player)) {
                queue_request<request_characterchoice>(&p);
            }
        } else {
            for (player &p : m_players) {
                add_log("LOG_CHARACTER_CHOICE", &p, p.m_characters.front());
                send_card_update(p.m_characters.front(), &p, show_card_flags::instant | show_card_flags::shown);
                p.reset_max_hp();
                p.set_hp(p.m_max_hp, true);
                p.m_characters.front()->on_enable(&p);

                move_card(p.m_characters.back(), pocket_type::player_backup, &p, show_card_flags::instant | show_card_flags::hidden);
            }
        }

        queue_action([this] {
            add_log("LOG_GAME_START");
            play_sound(nullptr, "gamestart");

            for (player &p : m_players) {
                p.m_characters.front()->on_equip(&p);
            }

            for (player &p : range_all_players(m_first_player,
                std::ranges::max(m_players | std::views::transform(&player::get_initial_cards))))
            {
                if (p.m_hand.size() < p.get_initial_cards()) {
                    p.draw_card();
                }
            }

            if (!m_shop_deck.empty()) {
                for (int i=0; i<3; ++i) {
                    draw_shop_card();
                }
            }

            if (!m_scenario_deck.empty()) {
                send_card_update(m_scenario_deck.back(), nullptr, show_card_flags::instant);
            }

            m_playing = m_first_player;
            m_first_player->start_of_turn();
        });
    }

    request_status_args game::make_request_update(player *p) {
        const auto &req = top_request();
        request_status_args ret{
            req.origin_card(),
            req.origin(),
            req.target(),
            req.status_text(p),
            req.flags()
        };

        ret.highlight_cards = to_vector_not_null(req.get_highlights());

        if (!p) return ret;

        auto add_ids_for = [&](auto &&cards) {
            for (card *c : cards) {
                if (req.can_respond(p, c)) {
                    ret.respond_cards.push_back(c);
                }
            }
        };

        add_ids_for(p->m_hand | std::views::filter([](card *c) { return c->color == card_color_type::brown; }));
        add_ids_for(p->m_table | std::views::filter(std::not_fn(&card::inactive)));
        add_ids_for(p->m_characters);
        add_ids_for(m_scenario_cards | std::views::reverse | std::views::take(1));
        add_ids_for(m_button_row);
        
        if (bool(req.flags() & effect_flags::force_play)) {
            add_ids_for(m_shop_selection);
        }

        if (req.target() != p) return ret;

        auto maybe_add_pick_id = [&](card *target_card) {
            if (req.can_pick(target_card)) {
                ret.pick_cards.emplace_back(target_card);
            }
        };

        for (player &target : m_players) {
            std::ranges::for_each(target.m_hand, maybe_add_pick_id);
            std::ranges::for_each(target.m_table, maybe_add_pick_id);
            std::ranges::for_each(target.m_characters, maybe_add_pick_id);
        }
        std::ranges::for_each(m_selection, maybe_add_pick_id);

        if (!m_deck.empty()) {
            maybe_add_pick_id(m_deck.back());
        }
        if (!m_discards.empty()) {
            maybe_add_pick_id(m_discards.back());
        }

        return ret;
    }

    void game::send_request_status_clear() {
        add_update<game_update_type::status_clear>();
    }

    void game::send_request_update() {
        auto spectator_target = update_target::excludes_public();
        for (player &p : m_players) {
            spectator_target.add(&p);
            add_update<game_update_type::request_status>(update_target::includes_private(&p), make_request_update(&p));
        }
        add_update<game_update_type::request_status>(std::move(spectator_target), make_request_update(nullptr));
    }
    
    void game::draw_check_then(player *origin, card *origin_card, draw_check_function fun) {
        flash_card(origin_card);
        m_current_check.set(origin, origin_card, std::move(fun));
        m_current_check.start();
    }

    void game::start_next_turn() {
        auto it = m_players.find(m_playing->id);
        do {
            if (check_flags(game_flags::invert_rotation)) {
                if (it == m_players.begin()) it = m_players.end();
                --it;
            } else {
                ++it;
                if (it == m_players.end()) it = m_players.begin();
            }
            call_event<event_type::verify_revivers>(&*it);
        } while (!it->alive());

        player *next_player = &*it;
        
        if (next_player == m_first_player) {
            queue_action_front([this]{
                draw_scenario_card();
            });
        }

        queue_action_front([next_player]{
            next_player->start_of_turn();
        });
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
        
        queue_action_front([this, killer, target, reason]{
            add_update<game_update_type::player_show_role>(target, target->m_role);
            target->add_player_flags(player_flags::role_revealed | player_flags::dead);
            target->set_hp(0, true);

            call_event<event_type::on_player_death>(killer, target);
        
            queue_action_front([=]{
                target->discard_all(reason);
            });
        });

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
                winner_role = alive_players_view.front().m_role;
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
            
            if (target == m_first_player && num_alive > 1 && winner_role == player_role::unknown) {
                m_first_player = std::next(player_iterator(m_first_player));
                add_update<game_update_type::move_scenario_deck>(m_first_player);
            }

            if (winner_role == player_role::unknown && !bool(m_options.expansions & card_expansion_type::ghostcards)) {
                target->add_player_flags(player_flags::removed);
                add_update<game_update_type::player_remove>(target);
            }

            if (winner_role != player_role::unknown) {
                for (player &p : m_players) {
                    if (!p.check_player_flags(player_flags::role_revealed)) {
                        add_update<game_update_type::player_show_role>(&p, p.m_role);
                    }
                }
                add_log("LOG_GAME_OVER");
                set_game_flags(game_flags::game_over);
                add_update<game_update_type::game_over>(winner_role);
            } else if (m_playing == target) {
                start_next_turn();
            } else if (killer) {
                if (m_players.size() > 3) {
                    switch (target->m_role) {
                    case player_role::outlaw:
                        killer->draw_card(3);
                        break;
                    case player_role::deputy:
                        if (killer->m_role == player_role::sheriff) {
                            queue_action([this, killer]{
                                add_log("LOG_SHERIFF_KILLED_DEPUTY", killer);
                                killer->discard_all(discard_all_reason::sheriff_killed_deputy);
                            });
                        }
                        break;
                    }
                } else {
                    killer->draw_card(3);
                }
            }
        });
    }

}