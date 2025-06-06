#include "game.h"

#include "game_update.h"
#include "game_options.h"

#include "cards/bang_cards.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "effects/base/requests.h"
#include "effects/base/death.h"

#include "play_verify.h"
#include "possible_to_play.h"

#include <array>
#include <unordered_set>

namespace banggame {

    static game_updates::preload_assets make_preload_assets_update(game_ptr table) {
        std::unordered_set<std::string> images;
        for (const card_data &card : table->m_cards_storage) {
            auto image = card.image.substr(0, card.image.find(':'));
            if (!image.empty()) {
                if (image.contains('/')) {
                    images.emplace(image);
                } else {
                    images.emplace(std::format("{}/{}", enums::to_string(card.deck), image));
                }
            }
            switch (card.deck) {
            case card_deck_type::none:
            case card_deck_type::locomotive:
            case card_deck_type::legends:
                break;
            default:
                images.emplace(std::format("backface/{}", enums::to_string(card.deck)));
            }
        }
        for (player_role role : enums::enum_values<player_role>()) {
            if (role == player_role::unknown) {
                images.emplace("backface/role");
            } else {
                images.emplace(std::format("role/{}", enums::to_string(role)));
            }
        }

        std::unordered_set<sound_id> sounds = bang_cards.expansions
            | rv::filter([&](const expansion_data &data) {
                return !data.expansion || table->m_options.expansions.contains(data.expansion);
            })
            | rv::for_each(&expansion_data::sounds)
            | rn::to<std::unordered_set>;
        
        return {
            .images {images | rn::to_vector},
            .sounds {sounds | rn::to_vector}
        };
    }

    static std::optional<game_updates::timer_status> get_request_timer_status(request_base &req) {
        if (auto *timer = dynamic_cast<request_timer *>(&req); timer && timer->enabled()) {
            return game_updates::timer_status{
                .timer_id = timer->get_timer_id(),
                .duration = timer->get_duration()
            };
        }
        return std::nullopt;
    }

    static game_updates::player_distances make_player_distances(player_ptr owner) {
        if (!owner) return {};

        return {
            .distance_mods = owner->m_game->m_players
                | rv::filter([&](player_ptr target) { return target != owner; })
                | rv::transform([&](player_ptr target) {
                    return game_updates::player_distance_item {
                        .player = target,
                        .value = target->get_distance_mod()
                    };
                })
                | rv::cache1
                | rv::filter([](const game_updates::player_distance_item &item) {
                    return item.value != 0;
                })
                | rn::to_vector,
            .range_mod = owner->get_range_mod(),
            .weapon_range = owner->get_weapon_range()
        };
    }
    
    static player_list get_request_target_set_players(player_ptr origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set_players>(target_is{origin})) {
                return origin->m_game->m_players
                    | rv::filter([&](const_player_ptr p){ return req->in_target_set(p); })
                    | rn::to_vector;
            }
        }
        return {};
    }

    static card_list get_request_target_set_cards(player_ptr origin) {
        if (origin) {
            if (auto req = origin->m_game->top_request<interface_target_set_cards>(target_is{origin})) {
                return get_all_targetable_cards(origin)
                    | rv::filter([&](const_card_ptr c){ return req->in_target_set(c); })
                    | rn::to_vector;
            }
        }
        return {};
    }

    static game_updates::request_status make_request_update(request_base &req, player_ptr owner = nullptr) {
        return game_updates::request_status {
            .origin_card = req.origin_card,
            .origin = req.origin,
            .target = req.target,
            .status_text = req.status_text(owner),
            .respond_cards = generate_playable_cards_list(owner, true),
            .highlight_cards = req.get_highlights(owner),
            .target_set_players = get_request_target_set_players(owner),
            .target_set_cards = get_request_target_set_cards(owner),
            .distances = make_player_distances(owner),
            .timer = get_request_timer_status(req)
        };
    }

    static game_updates::status_ready make_status_ready_update(player_ptr owner) {
        return {
            .play_cards = generate_playable_cards_list(owner),
            .distances = make_player_distances(owner)
        };
    }
    
    ticks game::get_total_update_time() const {
        game_duration result{0};
        for (player_ptr p : m_players) {
            game_duration player_result{0};
            for (const auto &[target, content, duration] : m_updates) {
                if (duration > game_duration{0} && target.matches(p)) {
                    player_result += duration;
                }
            }
            if (player_result > result) {
                result = player_result;
            }
        }
        return std::chrono::duration_cast<ticks>(transform_duration(result));
    }

    std::generator<game_update> game::get_spectator_join_updates() {
        co_yield make_preload_assets_update(this);

        co_yield game_updates::player_add{ m_players };

        for (player_ptr p : m_players) {
            co_yield game_updates::player_flags{ p, p->m_player_flags };
        }

        co_yield game_updates::player_order{ m_players, 0ms };

        auto add_cards = [&](pocket_type pocket, player_ptr owner = nullptr) -> std::generator<game_update> {
            auto &range = get_pocket(pocket, owner);
            if (!range.empty()) {
                co_yield game_updates::add_cards{ range, pocket, owner };
            }
            for (card_ptr c : range) {
                if (c->get_visibility() == card_visibility::shown) {
                    co_yield game_updates::show_card{ c, *c, 0ms };
                }
                for (const auto &[token, count] : c->tokens) {
                    if (count > 0) {
                        co_yield game_updates::add_tokens{ token, count, token_positions::card{c} };
                    }
                }
                if (c->inactive) {
                    co_yield game_updates::tap_card{ c, true, 0ms };
                }
            }
        };

        co_yield std::ranges::elements_of(add_cards(pocket_type::button_row));
        co_yield std::ranges::elements_of(add_cards(pocket_type::main_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::shop_deck));

        co_yield std::ranges::elements_of(add_cards(pocket_type::discard_pile));
        co_yield std::ranges::elements_of(add_cards(pocket_type::selection));
        co_yield std::ranges::elements_of(add_cards(pocket_type::shop_selection));
        co_yield std::ranges::elements_of(add_cards(pocket_type::hidden_deck));

        if (train_position != 0) {
            co_yield game_updates::move_train{ train_position, 0ms };
        }

        co_yield std::ranges::elements_of(add_cards(pocket_type::stations));
        co_yield std::ranges::elements_of(add_cards(pocket_type::train_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::train));

        co_yield std::ranges::elements_of(add_cards(pocket_type::scenario_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::scenario_card));
        co_yield std::ranges::elements_of(add_cards(pocket_type::wws_scenario_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::wws_scenario_card));

        co_yield std::ranges::elements_of(add_cards(pocket_type::feats_deck));
        co_yield std::ranges::elements_of(add_cards(pocket_type::feats_discard));
        co_yield std::ranges::elements_of(add_cards(pocket_type::feats));
        
        for (const auto &[token, count] : tokens) {
            if (count > 0) {
                co_yield game_updates::add_tokens{ token, count, token_positions::table{} };
            }
        }

        for (player_ptr p : m_players) {
            if (p->check_player_flags(player_flag::role_revealed)) {
                co_yield game_updates::player_show_role{ p, p->m_role, 0ms };
            }

            if (!p->check_player_flags(player_flag::removed)) {
                co_yield std::ranges::elements_of(add_cards(pocket_type::player_character, p));

                co_yield std::ranges::elements_of(add_cards(pocket_type::player_table, p));
                co_yield std::ranges::elements_of(add_cards(pocket_type::player_hand, p));

                co_yield game_updates::player_hp{ p, p->m_hp, 0ms };

                for (const auto &[token, count] : p->tokens) {
                    if (count > 0) {
                        co_yield game_updates::add_tokens{ token, count, token_positions::player{p} };
                    }
                }
            }
        }

        if (m_playing) {
            co_yield game_updates::switch_turn{ m_playing };
        }
        if (!is_waiting()) {
            if (auto req = top_request()) {
                co_yield make_request_update(*req);
            }
        }

        co_yield game_updates::game_flags{ m_game_flags };
    }

    std::generator<game_update> game::get_game_log_updates(player_ptr target) {
        co_yield game_updates::clear_logs{};
        
        for (const auto &[upd_target, log] : m_saved_log) {
            if (upd_target.matches(target)) {
                co_yield game_updates::game_log{ log };
            }
        }
    }

    std::generator<game_update> game::get_rejoin_updates(player_ptr target) {
        co_yield game_updates::player_add{ target };

        if (!target->check_player_flags(player_flag::role_revealed)) {
            co_yield game_updates::player_show_role{ target, target->m_role, 0ms };
        }

        if (!check_flags(game_flag::hands_shown)) {
            for (player_ptr p : m_players) {
                for (card_ptr c : p->m_hand) {
                    if (c->visibility.matches(target)) {
                        co_yield game_updates::show_card{ c, *c, 0ms };
                    }
                }
            }
        }

        for (card_ptr c : m_selection) {
            if (c->visibility.matches(target)) {
                co_yield game_updates::show_card{ c, *c, 0ms };
            }
        }

        if (!is_game_over() && !is_waiting()) {
            if (auto req = top_request()) {
                co_yield make_request_update(*req, target);
            } else if (target == m_playing) {
                co_yield make_status_ready_update(target);
            }
        }
    }

    static bool matches_expansions(const expansion_list &lhs, const expansion_set &rhs) {
        for (ruleset_ptr ruleset : lhs) {
            if (!rhs.contains(ruleset)) {
                return false;
            }
        }
        return true;
    }

    void game::start_game() {
        for (ruleset_ptr ruleset : m_options.expansions) {
            ruleset->on_apply(this);
        }

        add_update(game_updates::player_add{ m_players });
        
        auto add_cards = [&](auto &&cards, pocket_type pocket, card_list *add_to_list = nullptr) {
            if (!add_to_list && pocket != pocket_type::none) add_to_list = &get_pocket(pocket);

            int count = 0;
            for (const card_data &c : cards) {
                if (!matches_expansions(c.expansion, m_options.expansions)) {
                    continue;
                }

                card_ptr new_card = add_card(c);
                new_card->pocket = pocket;
                
                if (add_to_list) {
                    add_to_list->push_back(new_card);
                }
                ++count;
            }
            return count;
        };

        if (add_cards(bang_cards.button_row, pocket_type::button_row)) {
            add_update(game_updates::add_cards{ m_button_row, pocket_type::button_row });
            for (card_ptr c : m_button_row) {
                c->set_visibility(card_visibility::shown, nullptr, true);
            }
        }

        if (add_cards(bang_cards.hidden, pocket_type::hidden_deck)) {
            add_update(game_updates::add_cards{ m_hidden_deck, pocket_type::hidden_deck });
            for (card_ptr c : m_hidden_deck) {
                c->set_visibility(card_visibility::shown, nullptr, true);
            }
        }

        if (add_cards(bang_cards.deck, pocket_type::main_deck)) {
            shuffle_cards_and_ids(m_deck);
            add_update(game_updates::add_cards{ m_deck, pocket_type::main_deck });
        }

        if (add_cards(bang_cards.goldrush, pocket_type::shop_deck)) {
            shuffle_cards_and_ids(m_shop_deck);
            add_update(game_updates::add_cards{ m_shop_deck, pocket_type::shop_deck });
        }

        if (add_cards(bang_cards.train, pocket_type::train_deck)) {
            shuffle_cards_and_ids(m_train_deck);
            add_update(game_updates::add_cards{ m_train_deck, pocket_type::train_deck });
        }

        if (add_cards(bang_cards.feats, pocket_type::feats_deck)) {
            shuffle_cards_and_ids(m_feats_deck);
            add_update(game_updates::add_cards{ m_feats_deck, pocket_type::feats_deck });
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
            p->set_role(*role_ptr++, true);
        }

        m_first_player = *rn::find(m_players,
            m_players.size() > 3 ? player_role::sheriff : player_role::deputy_3p, &player::m_role);

        auto is_last_scenario_card = [](card_ptr c) {
            return c->has_tag(tag_type::last_scenario_card);
        };

        if (add_cards(bang_cards.highnoon, pocket_type::scenario_deck) + add_cards(bang_cards.fistfulofcards, pocket_type::scenario_deck)) {
            shuffle_cards_and_ids(m_scenario_deck);
            auto last_scenario_cards = rn::partition(m_scenario_deck, is_last_scenario_card);
            if (last_scenario_cards != m_scenario_deck.begin()) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, last_scenario_cards);
            }
            if (m_scenario_deck.size() > m_options.scenario_deck_size) {
                m_scenario_deck.erase(m_scenario_deck.begin() + 1, m_scenario_deck.end() - m_options.scenario_deck_size);
            }

            add_update(game_updates::add_cards{ m_scenario_deck, pocket_type::scenario_deck });
        }

        if (add_cards(bang_cards.wildwestshow, pocket_type::wws_scenario_deck)) {
            shuffle_cards_and_ids(m_wws_scenario_deck);
            rn::partition(m_wws_scenario_deck, is_last_scenario_card);
            add_update(game_updates::add_cards{ m_wws_scenario_deck, pocket_type::wws_scenario_deck });
        }

        add_cards(bang_cards.stations, pocket_type::none, &m_stations_deck);
        add_cards(bang_cards.locomotive, pocket_type::none, &m_locomotive);

        add_cards(bang_cards.legends, pocket_type::none, &m_legends);

        if (add_cards(bang_cards.characters | rv::filter([&](const card_data &c) {
            return !m_options.only_base_characters || c.expansion.empty();
        }), pocket_type::none, &m_characters)) {
            rn::shuffle(m_characters, rng);
        }
        
        add_update(make_preload_assets_update(this));

        add_game_flags(game_flag::hands_shown);

        auto character_it = m_characters.begin();
        int max_character_choice = m_characters.size() / num_alive();
        for (player_ptr p : range_alive_players(m_first_player)) {
            if (m_options.character_choice > 1) {
                card_list characters;
                characters.reserve(std::min(max_character_choice, m_options.character_choice));
                for (int i=0; i<characters.capacity(); ++i) {
                    characters.push_back(*character_it++);
                }

                add_cards_to(std::move(characters), pocket_type::player_hand, p, card_visibility::shown);
                queue_request<request_characterchoice>(p);
            } else {
                card_ptr target_card = *character_it++;
                add_log("LOG_CHARACTER_CHOICE", p, target_card);
                p->set_character(target_card);
                p->set_hp(p->get_character_max_hp(), true);
            }
        }

        queue_action([this] {
            remove_game_flags(game_flag::hands_shown);
            add_log("LOG_GAME_START");
            play_sound(sound_id::gamestart);

            auto players = range_alive_players(m_first_player);
            auto initial_cards = players
                | rv::transform([this](const_player_ptr p) {
                    int ncards = p->m_hp;
                    call_event(event_type::count_initial_cards{ p, ncards });
                    return ncards;
                })
                | rn::to_vector;

            int cycles = rn::max(initial_cards);
            for (int i=0; i<cycles; ++i) {
                for (const auto &[p, ncards] : rv::zip(players, initial_cards)) {
                    if (p->m_hand.size() < ncards) {
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

    request_state game::send_request_status_ready() {
        if (!m_playing) {
            return request_states::done{};
        }
        if (!m_playing->alive()) {
            start_next_turn();
            return request_states::next{};
        }

        auto args = make_status_ready_update(m_playing);
        
        if (m_playing->empty_hand() && rn::all_of(args.play_cards, [](const playable_card_info &args) {
            return args.card->has_tag(tag_type::pass_turn);
        })) {
            m_playing->pass_turn();
            return request_states::next{};
        } else {
            add_update(update_target::includes(m_playing), std::move(args));
            return request_states::done{};
        }
    }

    void game::send_request_update() {
        auto req = top_request();
        auto spectator_target = update_target::excludes();
        for (player_ptr p : m_players) {
            spectator_target.add(p);
            if (!p->is_bot()) {
                add_update(update_target::includes(p), make_request_update(*req, p));
            }
        }
        add_update(std::move(spectator_target), make_request_update(*req));
    }

}