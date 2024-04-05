#include "game.h"

#include "game_update.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/rulesets.h"
#include "effects/base/requests.h"
#include "effects/base/deathsave.h"

#include "play_verify.h"
#include "possible_to_play.h"

#include <array>

namespace banggame {

    game::game(unsigned int seed)
        : game_table(seed)
        , request_queue(this) {}

    player_user_pair::player_user_pair(player *p)
        : player_id{p->id}, user_id{p->user_id} {}

    player_order_update game::make_player_order_update(bool instant) {
        return player_order_update{m_players
            | rv::filter([](player *p) {
                return !p->check_player_flags(player_flags::removed);
            })
            | rn::to<serial::player_list>,
            instant};
    }

    util::generator<json::json> game::get_spectator_join_updates() {
        co_yield make_update<game_update_type::player_add>(m_players | rn::to<std::vector<player_user_pair>>);

        for (player *p : m_players) {
            co_yield make_update<game_update_type::player_flags>(p, p->m_player_flags);
        }

        co_yield make_update<game_update_type::player_order>(make_player_order_update(true));

        auto add_cards = [&](pocket_type pocket, player *owner = nullptr) -> util::generator<json::json> {
            auto &range = get_pocket(pocket, owner);
            if (!range.empty()) {
                co_yield make_update<game_update_type::add_cards>(range | rn::to<std::vector<card_backface>>, pocket, owner);
            }
            for (card *c : range) {
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

        co_await add_cards(pocket_type::button_row);
        co_await add_cards(pocket_type::main_deck);
        co_await add_cards(pocket_type::shop_deck);

        co_await add_cards(pocket_type::discard_pile);
        co_await add_cards(pocket_type::selection);
        co_await add_cards(pocket_type::shop_discard);
        co_await add_cards(pocket_type::shop_selection);
        co_await add_cards(pocket_type::hidden_deck);

        if (train_position != 0) {
            co_yield make_update<game_update_type::move_train>(train_position, true);
        }

        co_await add_cards(pocket_type::stations);
        co_await add_cards(pocket_type::train_deck);
        co_await add_cards(pocket_type::train);

        co_await add_cards(pocket_type::scenario_deck);
        co_await add_cards(pocket_type::scenario_card);
        co_await add_cards(pocket_type::wws_scenario_deck);
        co_await add_cards(pocket_type::wws_scenario_card);
        
        if (num_cubes > 0) {
            co_yield make_update<game_update_type::add_cubes>(num_cubes);
        }

        for (player *p : m_players) {
            if (p->check_player_flags(player_flags::role_revealed)) {
                co_yield make_update<game_update_type::player_show_role>(p, p->m_role, true);
            }

            if (!p->check_player_flags(player_flags::removed)) {
                co_await add_cards(pocket_type::player_character, p);
                co_await add_cards(pocket_type::player_backup, p);

                co_await add_cards(pocket_type::player_table, p);
                co_await add_cards(pocket_type::player_hand, p);

                co_yield make_update<game_update_type::player_hp>(p, p->m_hp, true);
                
                if (p->m_gold != 0) {
                    co_yield make_update<game_update_type::player_gold>(p, p->m_gold);
                }
            }
        }

        if (m_playing) {
            co_yield make_update<game_update_type::switch_turn>(m_playing);
        }
        if (!is_waiting() && pending_requests()) {
            co_yield make_update<game_update_type::request_status>(make_request_update(nullptr));
        }

        co_yield make_update<game_update_type::game_flags>(m_game_flags);
    }

    util::generator<json::json> game::get_game_log_updates(player *target) {
        co_yield make_update<game_update_type::clear_logs>();
        
        for (const auto &[upd_target, log] : m_saved_log) {
            if (upd_target.matches(target)) {
                co_yield make_update<game_update_type::game_log>(log);
            }
        }
    }

    util::generator<json::json> game::get_rejoin_updates(player *target) {
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

        if (!is_game_over() && !is_waiting()) {
            if (pending_requests()) {
                co_yield make_update<game_update_type::request_status>(make_request_update(target));
            } else if (target == m_playing) {
                co_yield make_update<game_update_type::status_ready>(make_status_ready_update(target));
            }
        }
    }

    void game::add_players(std::span<int> user_ids) {
        rn::shuffle(user_ids, rng);

        int player_id = 0;
        for (int id : user_ids) {
            player &p = m_context.players.emplace(this, ++player_id);
            p.user_id = id;
            m_players.emplace_back(&p);
        }
    }

    card_sign game::get_card_sign(card *target_card) {
        auto sign = target_card->sign;
        call_event(event_type::apply_sign_modifier{ sign });
        return sign;
    }

    void game::start_game(const game_options &options) {
        m_options = options;

        apply_rulesets(this);

        add_update<game_update_type::player_add>(m_players | rn::to<std::vector<player_user_pair>>);

        bool ghost_card_added = false;
        
        auto add_cards = [&](const std::vector<card_data> &cards, pocket_type pocket, std::vector<card *> *out_pocket = nullptr) {
            if (!out_pocket && pocket != pocket_type::none) out_pocket = &get_pocket(pocket);

            int count = 0;
            for (const card_data &c : cards) {
                if ((c.expansion & m_options.expansions) != c.expansion) continue;

                if (c.has_tag(tag_type::ghost_card)) {
                    if (m_options.enable_ghost_cards) {
                        ghost_card_added = true;
                    } else {
                        continue;
                    }
                }

                card *new_card = add_card(c);
                new_card->pocket = pocket;
                
                if (out_pocket) {
                    out_pocket->push_back(new_card);
                }
                ++count;
            }
            return count;
        };

        if (add_cards(all_cards.button_row, pocket_type::button_row)) {
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_button_row), pocket_type::button_row);
            for (card *c : m_button_row) {
                set_card_visibility(c, nullptr, card_visibility::shown, true);
            }
        }

        if (add_cards(all_cards.hidden, pocket_type::hidden_deck)) {
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_hidden_deck), pocket_type::hidden_deck);
            for (card *c : m_hidden_deck) {
                set_card_visibility(c, nullptr, card_visibility::shown, true);
            }
        }

        if (add_cards(all_cards.deck, pocket_type::main_deck)) {
            shuffle_cards_and_ids(m_deck);
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_deck), pocket_type::main_deck);
        }

        if (add_cards(all_cards.goldrush, pocket_type::shop_deck)) {
            shuffle_cards_and_ids(m_shop_deck);
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_shop_deck), pocket_type::shop_deck);
        }

        if (add_cards(all_cards.train, pocket_type::train_deck)) {
            shuffle_cards_and_ids(m_train_deck);
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_train_deck), pocket_type::train_deck);
        }
        
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

        rn::shuffle(role_ptr, role_ptr + m_players.size(), rng);
        for (player *p : m_players) {
            p->set_role(*role_ptr++);
        }

        m_first_player = *rn::find(m_players,
            m_players.size() > 3 ? player_role::sheriff : player_role::deputy_3p, &player::m_role);

        auto is_last_scenario_card = [](card *c) {
            return c->has_tag(tag_type::last_scenario_card);
        };

        if (add_cards(all_cards.highnoon, pocket_type::scenario_deck) + add_cards(all_cards.fistfulofcards, pocket_type::scenario_deck)) {
            shuffle_cards_and_ids(m_scenario_deck);
            auto last_scenario_cards = rn::partition(m_scenario_deck, is_last_scenario_card);
            if (last_scenario_cards != m_scenario_deck.begin()) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, last_scenario_cards);
            }
            if (m_scenario_deck.size() > m_options.scenario_deck_size) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, m_scenario_deck.end() - m_options.scenario_deck_size);
            }

            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_scenario_deck), pocket_type::scenario_deck);
        }

        if (add_cards(all_cards.wildwestshow, pocket_type::wws_scenario_deck)) {
            shuffle_cards_and_ids(m_wws_scenario_deck);
            rn::partition(m_wws_scenario_deck, is_last_scenario_card);
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(m_wws_scenario_deck), pocket_type::wws_scenario_deck);
        }

        add_cards(all_cards.stations, pocket_type::none);
        add_cards(all_cards.locomotive, pocket_type::none);

        std::vector<card *> character_ptrs;
        if (add_cards(all_cards.characters, pocket_type::none, &character_ptrs)) {
            rn::shuffle(character_ptrs, rng);
        }

        m_options.enable_ghost_cards = ghost_card_added;

        add_game_flags(game_flags::hands_shown);

        auto character_it = character_ptrs.rbegin();
        for (player *p : range_all_players(m_first_player)) {
            for (int i=0; i<2; ++i) {
                card *c = *character_it++;
                p->m_hand.push_back(c);
                c->pocket = pocket_type::player_hand;
                c->owner = p;
            }
            add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(p->m_hand), pocket_type::player_hand, p);
            for (card *c : p->m_hand) {
                set_card_visibility(c, p, card_visibility::shown, true);
            }
            queue_request<request_characterchoice>(p);
        }

        queue_action([this] {
            remove_game_flags(game_flags::hands_shown);
            add_log("LOG_GAME_START");
            play_sound(nullptr, "gamestart");

            int cycles = rn::max(m_players | rv::transform(&player::get_initial_cards));
            for (int i=0; i<cycles; ++i) {
                for (player *p : range_all_players(m_first_player)) {
                    if (p->m_hand.size() < p->get_initial_cards()) {
                        p->draw_card();
                    }
                }
            }

            call_event(event_type::on_game_setup{ m_first_player });
        });

        queue_action([this]{
            start_next_turn();
        });
    }

    player_distances game::make_player_distances(player *owner) {
        if (!owner) return {};

        return {
            .distances = m_players
                | rv::filter([&](player *target) { return target != owner; })
                | rv::transform([&](player *target) {
                    return player_distance_item {
                        .player = target,
                        .distance = target->get_distance_mod()
                    };
                })
                | rn::to_vector,
            .range_mod = owner->get_range_mod(),
            .weapon_range = owner->get_weapon_range()
        };
    }

    request_status_args game::make_request_update(player *owner) {
        auto req = top_request();
        auto *timer = req->timer();
        return request_status_args {
            .origin_card = req->origin_card,
            .origin = req->origin,
            .target = req->target,
            .status_text = req->status_text(owner),

            .respond_cards = generate_card_modifier_tree(owner, true),

            .pick_cards = rn::to<serial::card_list>(get_pick_cards(owner)),

            .highlight_cards = rn::to<serial::card_list>(req->get_highlights()),

            .target_set = rn::to<serial::player_list>(get_request_target_set(owner)),

            .distances = make_player_distances(owner),

            .timer = timer ? std::optional{timer_status_args{
                .timer_id = timer->get_timer_id(),
                .duration = std::chrono::duration_cast<game_duration>(timer->get_duration())
            }} : std::optional<timer_status_args>{}
        };
    }

    status_ready_args game::make_status_ready_update(player *owner) {
        return {
            .play_cards = generate_card_modifier_tree(owner),
            .distances = make_player_distances(owner)
        };
    }

    void game::send_request_status_clear() {
        add_update<game_update_type::status_clear>();
    }

    bool game::send_request_status_ready() {
        if (!m_playing->alive()) {
            start_next_turn();
            return false;
        }

        auto args = make_status_ready_update(m_playing);
        
        if (m_playing->empty_hand() && rn::all_of(args.play_cards, [](const card_modifier_node &node) {
            return node.card->has_tag(tag_type::pass_turn);
        })) {
            m_playing->pass_turn();
            return false;
        } else {
            add_update<game_update_type::status_ready>(update_target::includes_private(m_playing), std::move(args));
            return true;
        }
    }

    void game::send_request_update() {
        auto spectator_target = update_target::excludes_public();
        for (player *p : m_players) {
            spectator_target.add(p);
            if (!p->is_bot()) {
                add_update<game_update_type::request_status>(update_target::includes_private(p), make_request_update(p));
            }
        }
        add_update<game_update_type::request_status>(std::move(spectator_target), make_request_update(nullptr));
    }

    void game::start_next_turn() {
        if (num_alive() == 0) return;

        player *next_player;

        if (m_playing) {
            auto it = rn::find(m_players, m_playing);
            while (true) {
                if (check_flags(game_flags::invert_rotation)) {
                    if (it == m_players.begin()) it = m_players.end();
                    --it;
                } else {
                    ++it;
                    if (it == m_players.end()) it = m_players.begin();
                }
                if (!(*it)->remove_player_flags(player_flags::skip_turn)) {
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

    void game::handle_player_death(player *killer, player *target, discard_all_reason reason) {
        if (killer != m_playing) killer = nullptr;
        
        queue_action([this, killer, target, reason]{
            if (target->m_hp <= 0) {
                if (killer && killer != target) {
                    add_log("LOG_PLAYER_KILLED", killer, target);
                } else {
                    add_log("LOG_PLAYER_DIED", target);
                }

                target->add_player_flags(player_flags::dead);
                target->set_hp(0, true);
            }

            if (!target->alive()) {
                if (!m_first_dead) m_first_dead = target;

                target->remove_extra_characters();
                for (card *c : target->m_characters) {
                    target->disable_equip(c);
                }

                if (target->add_player_flags(player_flags::role_revealed)) {
                    add_update<game_update_type::player_show_role>(update_target::excludes(target), target, target->m_role);
                }

                if (reason != discard_all_reason::discard_ghost) {
                    call_event(event_type::on_player_death{ killer, target });
                }
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
                            queue_request<request_discard_all>(killer, discard_all_reason::sheriff_killed_deputy, -2);
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

        if (!m_options.enable_ghost_cards) {
            queue_action([this]{
                if (auto range = rv::filter(m_players, [](player *p) { return !p->alive() && !p->check_player_flags(player_flags::removed); })) {
                    for (player *p : range) {
                        p->add_player_flags(player_flags::removed);
                    }
                    
                    add_update<game_update_type::player_order>(make_player_order_update());
                }
            }, -3);
        }

        queue_action([this, killer, target] {
            if (target == m_first_player && !target->alive() && num_alive() > 1) {
                m_first_player = *std::next(player_iterator(target));
            }

            auto declare_winners = [this](auto &&winners) {
                for (player *p : range_all_players_and_dead(m_playing)) {
                    if (p->add_player_flags(player_flags::role_revealed)) {
                        add_update<game_update_type::player_show_role>(update_target::excludes(p), p, p->m_role);
                    }
                }
                add_log("LOG_GAME_OVER");
                for (player *p : winners) {
                    p->add_player_flags(player_flags::winner);
                }
                add_game_flags(game_flags::game_over);
            };

            auto alive_players = rv::filter(m_players, &player::alive);

            if (check_flags(game_flags::free_for_all)) {
                if (rn::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                }
            } else if (m_players.size() > 3) {
                auto is_outlaw = [](player *p) { return p->m_role == player_role::outlaw; };
                auto is_renegade = [](player *p) { return p->m_role == player_role::renegade; };
                auto is_sheriff = [](player *p) { return p->m_role == player_role::sheriff; };
                auto is_sheriff_or_deputy = [](player *p) { return p->m_role == player_role::sheriff || p->m_role == player_role::deputy; };

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
        }, -4);
    }

}