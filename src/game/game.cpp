#include "game.h"

#include "game_update.h"
#include "game_options.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "effects/base/requests.h"
#include "effects/base/deathsave.h"

#include "play_verify.h"
#include "possible_to_play.h"

#include <array>

namespace banggame {

    player_order_update game::make_player_order_update(bool instant) {
        return player_order_update{m_players
            | rv::filter([](player_ptr p) {
                return !p->check_player_flags(player_flag::removed);
            })
            | rn::to_vector,
            instant ? 0ms : durations.move_player};
    }
    
    ticks game::get_total_update_time() const {
        game_duration result{0};
        for (player_ptr p : m_players) {
            game_duration player_result{0};
            for (const auto &[target, content, duration] : m_updates) {
                if (duration >= game_duration{0} && target.matches(p)) {
                    player_result += duration;
                }
            }
            if (player_result > result) {
                result = player_result;
            }
        }
        return std::chrono::duration_cast<ticks>(transform_duration(result));
    }

    std::generator<json::json> game::get_spectator_join_updates() {
        co_yield make_update<"player_add">(m_players);

        for (player_ptr p : m_players) {
            co_yield make_update<"player_flags">(p, p->m_player_flags);
        }

        co_yield make_update<"player_order">(make_player_order_update(true));

        auto add_cards = [&](pocket_type pocket, player_ptr owner = nullptr) -> std::generator<json::json> {
            auto &range = get_pocket(pocket, owner);
            if (!range.empty()) {
                co_yield make_update<"add_cards">(range, pocket, owner);
            }
            for (card_ptr c : range) {
                if (c->visibility == card_visibility::shown) {
                    co_yield make_update<"show_card">(c, *c, 0ms);
                }
                if (c->num_cubes > 0) {
                    co_yield make_update<"add_cubes">(c->num_cubes, c);
                }
                if (c->inactive) {
                    co_yield make_update<"tap_card">(c, true, 0ms);
                }
            }
        };

        co_yield std::ranges::elements_of(add_cards(pocket_type::button_row));
        co_yield std::ranges::elements_of(add_cards(pocket_type::main_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::shop_deck));

        co_yield std::ranges::elements_of(add_cards(pocket_type::discard_pile));
        co_yield std::ranges::elements_of(add_cards(pocket_type::selection));
        co_yield std::ranges::elements_of(add_cards(pocket_type::shop_discard));
        co_yield std::ranges::elements_of(add_cards(pocket_type::shop_selection));
        co_yield std::ranges::elements_of(add_cards(pocket_type::hidden_deck));

        if (train_position != 0) {
            co_yield make_update<"move_train">(train_position, 0ms);
        }

        co_yield std::ranges::elements_of(add_cards(pocket_type::stations));
        co_yield std::ranges::elements_of(add_cards(pocket_type::train_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::train));

        co_yield std::ranges::elements_of(add_cards(pocket_type::scenario_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::scenario_card));
        co_yield std::ranges::elements_of(add_cards(pocket_type::wws_scenario_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::wws_scenario_card));
        
        if (num_cubes > 0) {
            co_yield make_update<"add_cubes">(num_cubes);
        }

        for (player_ptr p : m_players) {
            if (p->check_player_flags(player_flag::role_revealed)) {
                co_yield make_update<"player_show_role">(p, p->m_role, 0ms);
            }

            if (!p->check_player_flags(player_flag::removed)) {
                co_yield std::ranges::elements_of(add_cards(pocket_type::player_character, p));
                co_yield std::ranges::elements_of(add_cards(pocket_type::player_backup, p));

                co_yield std::ranges::elements_of(add_cards(pocket_type::player_table, p));
                co_yield std::ranges::elements_of(add_cards(pocket_type::player_hand, p));

                co_yield make_update<"player_hp">(p, p->m_hp, 0ms);
                
                if (p->m_gold != 0) {
                    co_yield make_update<"player_gold">(p, p->m_gold);
                }
            }
        }

        if (m_playing) {
            co_yield make_update<"switch_turn">(m_playing);
        }
        if (!is_waiting() && pending_requests()) {
            co_yield make_update<"request_status">(make_request_update(nullptr));
        }

        co_yield make_update<"game_flags">(m_game_flags);
    }

    std::generator<json::json> game::get_game_log_updates(player_ptr target) {
        co_yield make_update<"clear_logs">();
        
        for (const auto &[upd_target, log] : m_saved_log) {
            if (upd_target.matches(target)) {
                co_yield make_update<"game_log">(log);
            }
        }
    }

    std::generator<json::json> game::get_rejoin_updates(player_ptr target) {
        co_yield make_update<"player_add">(target);

        if (!target->check_player_flags(player_flag::role_revealed)) {
            co_yield make_update<"player_show_role">(target, target->m_role, 0ms);
        }

        for (card_ptr c : target->m_hand) {
            co_yield make_update<"show_card">(c, *c, 0ms);
        }

        for (card_ptr c : m_selection) {
            if (c->owner == target) {
                co_yield make_update<"show_card">(c, *c, 0ms);
            }
        }

        if (!is_game_over() && !is_waiting()) {
            if (pending_requests()) {
                co_yield make_update<"request_status">(make_request_update(target));
            } else if (target == m_playing) {
                co_yield make_update<"status_ready">(make_status_ready_update(target));
            }
        }
    }

    card_ptr game::add_card(const card_data &data) {
        return &m_cards_storage.emplace(this, int(m_cards_storage.first_available_id()), data);
    }

    void game::add_players(std::span<int> user_ids) {
        rn::shuffle(user_ids, rng);

        int player_id = 0;
        for (int id : user_ids) {
            m_players.emplace_back(&m_players_storage.emplace(this, ++player_id, id));
        }
    }

    static bool matches_expansions(const expansion_set &lhs, const expansion_set &rhs) {
        for (const ruleset_vtable *ruleset : lhs) {
            if (!rhs.contains(ruleset)) {
                return false;
            }
        }
        return true;
    }

    void game::start_game() {
        for (const ruleset_vtable *ruleset : m_options.expansions) {
            ruleset->on_apply(this);
        }

        add_update<"player_add">(m_players);
        
        auto add_cards = [&](std::span<const card_data> cards, pocket_type pocket, card_list *out_pocket = nullptr) {
            if (!out_pocket && pocket != pocket_type::none) out_pocket = &get_pocket(pocket);

            int count = 0;
            for (const card_data &c : cards) {
                if (!matches_expansions(c.expansion, m_options.expansions)) {
                    continue;
                }

                if (c.has_tag(tag_type::ghost_card) && !m_options.enable_ghost_cards) {
                    continue;
                }

                card_ptr new_card = add_card(c);
                new_card->pocket = pocket;
                
                if (out_pocket) {
                    out_pocket->push_back(new_card);
                }
                ++count;
            }
            return count;
        };

        if (add_cards(all_cards.button_row, pocket_type::button_row)) {
            add_update<"add_cards">(m_button_row, pocket_type::button_row);
            for (card_ptr c : m_button_row) {
                c->set_visibility(card_visibility::shown, nullptr, true);
            }
        }

        if (add_cards(all_cards.hidden, pocket_type::hidden_deck)) {
            add_update<"add_cards">(m_hidden_deck, pocket_type::hidden_deck);
            for (card_ptr c : m_hidden_deck) {
                c->set_visibility(card_visibility::shown, nullptr, true);
            }
        }

        if (add_cards(all_cards.deck, pocket_type::main_deck)) {
            shuffle_cards_and_ids(m_deck);
            add_update<"add_cards">(m_deck, pocket_type::main_deck);
        }

        if (add_cards(all_cards.goldrush, pocket_type::shop_deck)) {
            shuffle_cards_and_ids(m_shop_deck);
            add_update<"add_cards">(m_shop_deck, pocket_type::shop_deck);
        }

        if (add_cards(all_cards.train, pocket_type::train_deck)) {
            shuffle_cards_and_ids(m_train_deck);
            add_update<"add_cards">(m_train_deck, pocket_type::train_deck);
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
        for (player_ptr p : m_players) {
            p->set_role(*role_ptr++);
        }

        m_first_player = *rn::find(m_players,
            m_players.size() > 3 ? player_role::sheriff : player_role::deputy_3p, &player::m_role);

        auto is_last_scenario_card = [](card_ptr c) {
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

            add_update<"add_cards">(m_scenario_deck, pocket_type::scenario_deck);
        }

        if (add_cards(all_cards.wildwestshow, pocket_type::wws_scenario_deck)) {
            shuffle_cards_and_ids(m_wws_scenario_deck);
            rn::partition(m_wws_scenario_deck, is_last_scenario_card);
            add_update<"add_cards">(m_wws_scenario_deck, pocket_type::wws_scenario_deck);
        }

        add_cards(all_cards.stations, pocket_type::none);
        add_cards(all_cards.locomotive, pocket_type::none);

        card_list character_ptrs;
        if (add_cards(all_cards.characters, pocket_type::none, &character_ptrs)) {
            rn::shuffle(character_ptrs, rng);
        }

        add_game_flags(game_flag::hands_shown);

        auto character_it = character_ptrs.rbegin();
        for (player_ptr p : range_alive_players(m_first_player)) {
            for (int i=0; i<2; ++i) {
                card_ptr c = *character_it++;
                p->m_hand.push_back(c);
                c->pocket = pocket_type::player_hand;
                c->owner = p;
            }
            add_update<"add_cards">(p->m_hand, pocket_type::player_hand, p);
            if (m_options.character_choice) {
                for (card_ptr c : p->m_hand) {
                    c->set_visibility(card_visibility::shown, p, true);
                }
            }
            queue_request<request_characterchoice>(p);
        }

        queue_action([this] {
            remove_game_flags(game_flag::hands_shown);
            add_log("LOG_GAME_START");
            play_sound("gamestart");

            int cycles = rn::max(m_players | rv::transform(&player::get_initial_cards));
            for (int i=0; i<cycles; ++i) {
                for (player_ptr p : range_alive_players(m_first_player)) {
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

    player_distances game::make_player_distances(player_ptr owner) {
        if (!owner) return {};

        return {
            .distance_mods = m_players
                | rv::filter([&](player_ptr target) { return target != owner; })
                | rv::transform([&](player_ptr target) {
                    return player_distance_item {
                        .player = target,
                        .value = target->get_distance_mod()
                    };
                })
                | rv::filter([](const player_distance_item &item) {
                    return item.value != 0;
                })
                | rn::to_vector,
            .range_mod = owner->get_range_mod(),
            .weapon_range = owner->get_weapon_range()
        };
    }
    
    static player_list get_request_target_set_players(player_ptr origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set_players>(origin)) {
                return origin->m_game->m_players
                    | rv::filter([&](const_player_ptr p){ return req->in_target_set(p); })
                    | rn::to_vector;
            }
        }
        return {};
    }

    static card_list get_request_target_set_cards(player_ptr origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set_cards>(origin)) {
                return get_all_targetable_cards(origin)
                    | rv::filter([&](const_card_ptr c){ return req->in_target_set(c); })
                    | rn::to_vector;
            }
        }
        return {};
    }

    static std::optional<timer_status_args> get_request_timer_status(request_timer *timer) {
        if (timer) {
            return timer_status_args{
                .timer_id = timer->get_timer_id(),
                .duration = std::chrono::duration_cast<game_duration>(timer->get_duration())
            };
        }
        return std::nullopt;
    }

    request_status_args game::make_request_update(player_ptr owner) {
        auto req = top_request();
        return request_status_args{
            .origin_card = req->origin_card,
            .origin = req->origin,
            .target = req->target,
            .status_text = req->status_text(owner),
            .respond_cards = generate_playable_cards_list(owner, true),
            .highlight_cards = req->get_highlights(),
            .target_set_players = get_request_target_set_players(owner),
            .target_set_cards = get_request_target_set_cards(owner),
            .distances = make_player_distances(owner),
            .timer = get_request_timer_status(req->timer())
        };
    }

    status_ready_args game::make_status_ready_update(player_ptr owner) {
        return {
            .play_cards = generate_playable_cards_list(owner),
            .distances = make_player_distances(owner)
        };
    }

    void game::send_request_status_clear() {
        add_update<"status_clear">();
    }

    request_state game::send_request_status_ready() {
        if (!m_playing) {
            return utils::tag<"done">{};
        }
        if (!m_playing->alive()) {
            start_next_turn();
            return utils::tag<"next">{};
        }

        auto args = make_status_ready_update(m_playing);
        
        if (m_playing->empty_hand() && rn::all_of(args.play_cards, [](const playable_card_info &args) {
            return args.card->has_tag(tag_type::pass_turn);
        })) {
            m_playing->pass_turn();
            return utils::tag<"next">{};
        } else {
            add_update<"status_ready">(update_target::includes_private(m_playing), std::move(args));
            return utils::tag<"done">{};
        }
    }

    void game::send_request_update() {
        auto spectator_target = update_target::excludes_public();
        for (player_ptr p : m_players) {
            spectator_target.add(p);
            if (!p->is_bot()) {
                add_update<"request_status">(update_target::includes_private(p), make_request_update(p));
            }
        }
        add_update<"request_status">(std::move(spectator_target), make_request_update(nullptr));
    }

    void game::start_next_turn() {
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

    void game::handle_player_death(player_ptr killer, player_ptr target, discard_all_reason reason) {
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

        if (rn::none_of(get_all_cards(), [](const_card_ptr c) { return c->has_tag(tag_type::ghost_card); })) {
            queue_action([this]{
                bool any_player_removed = false;
                for (player_ptr p : m_players) {
                    if (!p->alive() && p->add_player_flags(player_flag::removed)) {
                        any_player_removed = true;
                    }
                }
                
                if (any_player_removed) {
                    add_update<"player_order">(make_player_order_update());
                }
            }, -3);
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
        }, -4);
    }

}