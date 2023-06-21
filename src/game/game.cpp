#include "game.h"

#include "game_update.h"

#include "cards/filters.h"
#include "cards/holders.h"
#include "cards/effect_enums.h"
#include "cards/game_enums.h"

#include "cards/base/requests.h"
#include "cards/greattrainrobbery/next_stop.h"

#include "play_verify.h"
#include "possible_to_play.h"
#include "draw_check_handler.h"

#include <array>

namespace banggame {

    json::json game_net_manager::serialize_update(const game_update &update) const {
        return json::serialize(update, context());
    }

    player_user_pair::player_user_pair(player *p)
        : player_id{p->id}, user_id{p->user_id} {}

    player_order_update game::make_player_order_update(bool instant) {
        return player_order_update{m_players
            | ranges::views::filter([](player *p) {
                return !p->check_player_flags(player_flags::removed);
            })
            | ranges::to<serial::player_list>,
            instant};
    }

    util::generator<json::json> game::get_spectator_updates() {
        co_yield make_update<game_update_type::player_add>(m_players | ranges::to<std::vector<player_user_pair>>);

        for (player *p : m_players) {
            co_yield make_update<game_update_type::player_status>(p, p->m_player_flags, p->m_range_mod, p->m_weapon_range, p->m_distance_mod);
        }

        co_yield make_update<game_update_type::player_order>(make_player_order_update(true));

        auto add_cards = [&](pocket_type pocket, player *owner = nullptr) -> util::generator<json::json> {
            auto &range = get_pocket(pocket, owner);
            if (!range.empty()) {
                co_yield make_update<game_update_type::add_cards>(range | ranges::to<std::vector<card_backface>>, pocket, owner);
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

        if (m_scenario_holder) {
            co_yield make_update<game_update_type::move_scenario_deck>(m_scenario_holder, pocket_type::scenario_deck, true);
        }
        if (m_wws_scenario_holder) {
            co_yield make_update<game_update_type::move_scenario_deck>(m_wws_scenario_holder, pocket_type::wws_scenario_deck, true);
        }

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
        if (pending_requests()) {
            co_yield make_update<game_update_type::request_status>(make_request_update(nullptr));
        }

        co_yield make_update<game_update_type::game_flags>(m_game_flags);
        
        for (const auto &[target, log] : m_saved_log) {
            if (target.matches(nullptr)) {
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

        if (pending_requests()) {
            co_yield make_update<game_update_type::request_status>(make_request_update(target));
        } else if (target == m_playing) {
            co_yield make_update<game_update_type::status_ready>(make_status_ready_update(target));
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

    card_sign game::get_card_sign(card *target_card) {
        return call_event<event_type::apply_sign_modifier>(target_card->sign);
    }

    void game::start_game(const game_options &options) {
        m_options = options;

        apply_rulesets(this);

        add_update<game_update_type::player_add>(m_players | ranges::to<std::vector<player_user_pair>>);
        
        auto add_cards = [&](const std::vector<card_data> &cards, pocket_type pocket, std::vector<card *> *out_pocket = nullptr) {
            if (!out_pocket && pocket != pocket_type::none) out_pocket = &get_pocket(pocket);

            int count = 0;
            for (const card_data &c : cards) {
                if (m_players.size() <= 2 && c.has_tag(tag_type::discard_if_two_players)) continue;
                if (c.has_tag(tag_type::ghost_card) && !m_options.enable_ghost_cards) continue;
                if ((c.expansion & m_options.expansions) != c.expansion) continue;

                card *new_card = &m_context.cards.emplace(int(m_context.cards.first_available_id()), c);
                new_card->pocket = pocket;
                
                if (out_pocket) {
                    out_pocket->push_back(new_card);
                }
                ++count;
            }
            return count;
        };

        if (add_cards(all_cards.button_row, pocket_type::button_row)) {
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_button_row), pocket_type::button_row);
            for (card *c : m_button_row) {
                set_card_visibility(c, nullptr, card_visibility::shown, true);
            }
        }

        if (add_cards(all_cards.deck, pocket_type::main_deck)) {
            shuffle_cards_and_ids(m_deck);
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_deck), pocket_type::main_deck);
        }

        if (add_cards(all_cards.goldrush, pocket_type::shop_deck)) {
            shuffle_cards_and_ids(m_shop_deck);
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_shop_deck), pocket_type::shop_deck);
        }

        if (add_cards(all_cards.train, pocket_type::train_deck)) {
            shuffle_cards_and_ids(m_train_deck);
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_train_deck), pocket_type::train_deck);
        }

        if (add_cards(all_cards.hidden, pocket_type::hidden_deck)) {
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_hidden_deck), pocket_type::hidden_deck);
            for (card *c : m_hidden_deck) {
                set_card_visibility(c, nullptr, card_visibility::shown, true);
            }
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

        std::ranges::shuffle(role_ptr, role_ptr + m_players.size(), rng);
        for (player *p : m_players) {
            p->set_role(*role_ptr++);
        }

        player *first_player = *std::ranges::find(m_players,
            m_players.size() > 3 ? player_role::sheriff : player_role::deputy_3p, &player::m_role);

        auto is_last_scenario_card = [](card *c) {
            return c->has_tag(tag_type::last_scenario_card);
        };

        if (add_cards(all_cards.highnoon, pocket_type::scenario_deck) + add_cards(all_cards.fistfulofcards, pocket_type::scenario_deck)) {
            shuffle_cards_and_ids(m_scenario_deck);
            auto last_scenario_cards = std::ranges::partition(m_scenario_deck, is_last_scenario_card);
            if (last_scenario_cards.begin() != m_scenario_deck.begin()) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, last_scenario_cards.begin());
            }
            if (m_scenario_deck.size() > m_options.scenario_deck_size) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, m_scenario_deck.end() - m_options.scenario_deck_size);
            }

            m_scenario_holder = first_player;
            add_update<game_update_type::move_scenario_deck>(m_scenario_holder, pocket_type::scenario_deck, true);
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_scenario_deck), pocket_type::scenario_deck);
        }

        if (add_cards(all_cards.wildwestshow, pocket_type::wws_scenario_deck)) {
            shuffle_cards_and_ids(m_wws_scenario_deck);
            std::ranges::partition(m_wws_scenario_deck, is_last_scenario_card);
            m_wws_scenario_holder = first_player;
            add_update<game_update_type::move_scenario_deck>(m_wws_scenario_holder, pocket_type::wws_scenario_deck, true);
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(m_wws_scenario_deck), pocket_type::wws_scenario_deck);
        }

        if (add_cards(all_cards.stations, pocket_type::none) && add_cards(all_cards.locomotive, pocket_type::none)) {
            m_train_holder = first_player;
        }

        std::vector<card *> character_ptrs;
        if (add_cards(all_cards.characters, pocket_type::none, &character_ptrs)) {
            std::ranges::shuffle(character_ptrs, rng);
        }

        auto add_character_to = [&](card *c, player *p) {
            p->m_characters.push_back(c);
            c->pocket = pocket_type::player_character;
            c->owner = p;
            add_update<game_update_type::add_cards>(ranges::to<std::vector<card_backface>>(std::views::single(c)), pocket_type::player_character, p);
        };

        auto character_it = character_ptrs.rbegin();

        for (player *p : m_players) {
            add_character_to(*character_it++, p);
            add_character_to(*character_it++, p);
        }

        if (m_options.character_choice) {
            for (player *p : m_players) {
                while (!p->m_characters.empty()) {
                    move_card(p->first_character(), pocket_type::player_table, p, card_visibility::shown, true);
                }
            }
            for (player *p : range_all_players(first_player)) {
                queue_request<request_characterchoice>(p);
            }
        } else {
            for (player *p : m_players) {
                add_log("LOG_CHARACTER_CHOICE", p, p->first_character());
                set_card_visibility(p->first_character(), nullptr, card_visibility::shown, true);
                p->reset_max_hp();
                p->set_hp(p->m_max_hp, true);
                p->enable_equip(p->first_character());

                move_card(p->m_characters.back(), pocket_type::player_backup, p, card_visibility::hidden, true);
            }
        }

        queue_action([this, first_player] {
            add_log("LOG_GAME_START");
            play_sound(nullptr, "gamestart");

            int cycles = std::ranges::max(m_players | std::views::transform(&player::get_initial_cards));
            for (int i=0; i<cycles; ++i) {
                for (player *p : range_all_players(first_player)) {
                    if (p->m_hand.size() < p->get_initial_cards()) {
                        p->draw_card();
                    }
                }
            }
            
            call_event<event_type::on_game_setup>(first_player);

            for (player *p : m_players) {
                p->first_character()->on_equip(p);
            }

            if (!m_scenario_deck.empty()) {
                set_card_visibility(m_scenario_deck.back(), nullptr, card_visibility::shown);
            }
            if (!m_wws_scenario_deck.empty()) {
                set_card_visibility(m_wws_scenario_deck.back(), nullptr, card_visibility::shown);
            }

            m_playing = first_player;
            first_player->start_of_turn();
        });
    }

    request_status_args game::make_request_update(player *owner) {
        auto req = top_request();
        return request_status_args {
            .origin_card = req->origin_card,
            .origin = req->origin,
            .target = req->target,
            .status_text = req->status_text(owner),
            .auto_select = req->auto_select(),

            .respond_cards = owner ? generate_card_modifier_tree(owner, true) : card_modifier_tree{},

            .pick_cards = owner && req->target == owner
                ? ranges::views::concat(
                    m_players | ranges::views::for_each([](player *p) {
                        return ranges::views::concat(p->m_hand, p->m_table, p->m_characters);
                    }),
                    m_selection,
                    m_deck | ranges::views::take(1),
                    m_discards | ranges::views::take(1)
                )
                | ranges::views::filter([&](card *target_card) {
                    return req->can_pick(target_card);
                })
                | ranges::to<serial::card_list>
                : serial::card_list{},

            .highlight_cards = ranges::to<serial::card_list>(req->get_highlights())
        };
    }

    status_ready_args game::make_status_ready_update(player *owner) {
        return {
            .play_cards = generate_card_modifier_tree(owner)
        };
    }

    void game::send_request_status_clear() {
        add_update<game_update_type::status_clear>();
    }

    bool game::send_request_status_ready() {
        auto args = make_status_ready_update(m_playing);
        
        if (m_playing->empty_hand() && std::ranges::all_of(args.play_cards, [](const card_modifier_node &node) {
            return node.card->has_tag(tag_type::pass_turn);
        })) {
            m_playing->pass_turn();
            update();
            return false;
        } else if (!m_playing->is_bot()) {
            add_update<game_update_type::status_ready>(update_target::includes_private(m_playing), std::move(args));
        }
        return true;
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
    
    void game::draw_check_then(player *origin, card *origin_card, draw_check_condition condition, draw_check_function fun) {
        flash_card(origin_card);
        queue_request_front<request_check>(this, origin_card, origin, std::move(condition), std::move(fun));
    }

    void game::start_next_turn() {
        auto it = std::ranges::find(m_players, m_playing);
        while (true) {
            if (check_flags(game_flags::invert_rotation)) {
                if (it == m_players.begin()) it = m_players.end();
                --it;
            } else {
                ++it;
                if (it == m_players.end()) it = m_players.begin();
            }
            if (!(*it)->remove_player_flags(player_flags::skip_turn)) {
                call_event<event_type::check_revivers>(*it);
                if ((*it)->alive()) break;
            }
        }

        player *next_player = *it;
        
        if (next_player == m_train_holder) {
            effect_next_stop{}.on_play(nullptr, next_player);
        }
        if (next_player == m_scenario_holder) {
            draw_scenario_card();
        }

        queue_action([next_player]{
            next_player->start_of_turn();
        }, 1);
    }

    void game::handle_player_death(player *killer, player *target, discard_all_reason reason) {
        if (killer != m_playing) killer = nullptr;

        if (killer && killer != target) {
            add_log("LOG_PLAYER_KILLED", killer, target);
        } else {
            add_log("LOG_PLAYER_DIED", target);
        }
        
        queue_action([this, killer, target, reason]{
            target->add_player_flags(player_flags::dead);
            target->set_hp(0, true);

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
                    call_event<event_type::on_player_death>(killer, target);
                }
            }
        }, 3);

        if (killer && m_players.size() > 3 && reason != discard_all_reason::discard_ghost) {
            queue_action([this, killer, target] {
                if (killer->alive() && !target->alive()) {
                    if (target->m_role == player_role::outlaw) {
                        add_log("LOG_KILLED_OUTLAW", killer);
                        killer->draw_card(3);
                    } else if (target->m_role == player_role::deputy && killer->m_role == player_role::sheriff) {
                        queue_action([this, killer] {
                            add_log("LOG_SHERIFF_KILLED_DEPUTY", killer);
                            queue_request<request_discard_all>(killer, discard_all_reason::sheriff_killed_deputy);
                        }, -2);
                    }
                }
            }, 3);
        }
        
        queue_action([this, target, reason]{
            if (!target->alive()) {
                queue_request<request_discard_all>(target, reason);
            }
        }, 3);

        if (reason == discard_all_reason::disable_temp_ghost) {
            return;
        }

        queue_action([this, target]{
            if (num_alive() > 1) {
                player *next_player = *std::next(player_iterator(target));
                if (target == m_scenario_holder) {
                    m_scenario_holder = next_player;
                    add_update<game_update_type::move_scenario_deck>(m_scenario_holder, pocket_type::scenario_deck);
                }
                if (target == m_wws_scenario_holder) {
                    m_wws_scenario_holder = next_player;
                    add_update<game_update_type::move_scenario_deck>(m_wws_scenario_holder, pocket_type::wws_scenario_deck);
                }
                if (target == m_train_holder) {
                    m_train_holder = next_player;
                }
            }
        }, -3);

        if (!m_options.enable_ghost_cards) {
            queue_action([this]{
                if (auto range = std::views::filter(m_players, [](player *p) { return !p->alive() && !p->check_player_flags(player_flags::removed); })) {
                    for (player *p : range) {
                        p->add_player_flags(player_flags::removed);
                    }
                    
                    add_update<game_update_type::player_order>(make_player_order_update());
                }
            }, -3);
        }

        queue_action([this, killer, target] {
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

            auto alive_players = std::views::filter(m_players, &player::alive);

            if (check_flags(game_flags::free_for_all)) {
                if (std::ranges::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                }
            } else if (m_players.size() > 3) {
                auto is_outlaw = [](player *p) { return p->m_role == player_role::outlaw; };
                auto is_renegade = [](player *p) { return p->m_role == player_role::renegade; };
                auto is_sheriff = [](player *p) { return p->m_role == player_role::sheriff; };
                auto is_sheriff_or_deputy = [](player *p) { return p->m_role == player_role::sheriff || p->m_role == player_role::deputy; };

                if (std::ranges::none_of(alive_players, is_sheriff)) {
                    if (std::ranges::distance(alive_players) == 1 && is_renegade(alive_players.front())) {
                        declare_winners(alive_players);
                    } else {
                        declare_winners(std::views::filter(m_players, is_outlaw));
                    }
                } else if (std::ranges::all_of(alive_players, is_sheriff_or_deputy)) {
                    declare_winners(std::views::filter(m_players, is_sheriff_or_deputy));
                }
            } else {
                if (std::ranges::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                } else if (killer && !target->alive() && (
                    (target->m_role == player_role::outlaw_3p && killer->m_role == player_role::renegade_3p) ||
                    (target->m_role == player_role::renegade_3p && killer->m_role == player_role::deputy_3p) ||
                    (target->m_role == player_role::deputy_3p && killer->m_role == player_role::outlaw_3p)))
                {
                    declare_winners(std::views::single(killer));
                }
            }
        }, -4);

        queue_action([this, killer, target]{
            if (!target->alive()) {
                if (killer && killer != target && m_players.size() <= 3) {
                    killer->draw_card(3);
                }
                if (m_playing == target) {
                    target->skip_turn();
                }
            }
        }, -5);
    }

}