#include "game.h"
#include "filters.h"
#include "game_options.h"
#include "play_verify.h"
#include "possible_to_play.h"
#include "request_timer.h"

#include "net/bot_info.h"
#include "net/logging.h"

namespace banggame {

    static game_action generate_random_play(player_ptr origin, const playable_card_info &args) {
        game_action ret {
            .card = args.card,
            .effect_list = args.effect_list
        };
        effect_context ctx;
        
        for (const auto &[mod_card, mod_response] : args.modifiers) {
            auto &targets = ret.modifiers.emplace_back(mod_card, mod_response).targets;

            mod_card->get_modifier(mod_response).add_context(mod_card, origin, ctx);
            for (const effect_holder &holder : mod_card->get_effect_list(mod_response)) {
                const auto &target = targets.emplace_back(holder.random_target(mod_card, origin, ctx));
                holder.add_context(mod_card, origin, target, ctx);
            }
        }

        const effect_list &effects = args.card->get_effect_list(args.effect_list);
        for (const effect_holder &holder : effects) {
            const auto &target = ret.targets.emplace_back(holder.random_target(args.card, origin, ctx));
            holder.add_context(args.card, origin, target, ctx);
        }

        return ret;
    }

    template<rn::input_range Rules, typename Nodes, typename Rng>
        requires (rn::forward_range<Nodes> && rn::sized_range<Nodes>
            && std::predicate<rn::range_value_t<Rules>, rn::range_value_t<Nodes>>)
    rn::iterator_t<Nodes> get_selected_node(Rules &&rules, Nodes &nodes, Rng &rng) {
        for (const auto &rule : rules) {
            auto chosen = nodes.end();
            size_t count = 0;

            for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                if (rule(*it)) {
                    ++count;
                    std::uniform_int_distribution<size_t> dist(0, count - 1);
                    if (dist(rng) == 0) chosen = it;
                }
            }

            if (chosen != nodes.end()) {
                return chosen;
            }
        }
        std::uniform_int_distribution<size_t> dist(0, nodes.size() - 1);
        return rn::next(nodes.begin(), dist(rng));
    }

    static request_state execute_random_play(player_ptr origin, const bot_rule_list &bot_rules, const playable_cards_list &play_cards) {
        auto timer = origin->m_game->top_request<request_timer>();
        bool add_empty = timer && timer->enabled();
        
        std::vector<card_node> node_set;
        node_set.reserve(play_cards.size() * bot_info.settings.repeat_card_nodes + add_empty);

        for (int i=0; i < bot_info.settings.max_random_tries; ++i) {
            for (const playable_card_info &info : play_cards) {
                for (int j=0; j<bot_info.settings.repeat_card_nodes; ++j) {
                    node_set.push_back(&info);
                }
            }
            
            if (add_empty) {
                node_set.push_back(nullptr);
            }
            
            while (!node_set.empty()) {
                auto it = get_selected_node(bot_rules, node_set, origin->m_game->bot_rng);

                card_node selected_node = *it; 
                if (!selected_node) {
                    return request_states::done{};
                }

                auto last = std::prev(node_set.end());
                rn::iter_swap(it, last);
                node_set.erase(last, node_set.end());

                auto args = generate_random_play(origin, *selected_node);
                args.bypass_prompt =
                    (i >= bot_info.settings.bypass_empty_index && node_set.empty())
                    || i >= bot_info.settings.bypass_unconditional_index;

                auto result = verify_and_play(origin, args);

                if (std::visit(overloaded{
                    [](play_verify_results::ok) {
                        return true;
                    },
                    [&](play_verify_results::prompt prompt) {
                        logging::trace("BOT PROMPT: message={}, i={}", std::string_view{prompt.message.message.format_str}, i);
                        return false;
                    },
                    [&](play_verify_results::error error) {
                        logging::trace("BOT ERROR: message={}, i={}", std::string_view{error.message.format_str}, i);
                        return false;
                    }
                }, result)) {
                    return request_states::next{};
                }
            }
        }

        // softlock
        logging::warn("BOT ERROR: could not find card in execute_random_play()");

        return request_states::done{};
    }

    request_state game::request_bot_play(bool instant) {
        if (rn::none_of(m_players, &player::is_bot)) {
            return request_states::done{};
        } else if (!instant && m_options.bot_play_timer > game_duration{0}) {
            return request_states::bot_play{ get_total_update_time() + std::chrono::duration_cast<ticks>(m_options.bot_play_timer) };
        }

        if (pending_requests()) {
            for (player_ptr origin : m_players | rv::filter(&player::is_bot)) {
                playable_cards_list play_cards = generate_playable_cards_list(origin, effect_list_type::responses);
                
                if (!play_cards.empty() && std::holds_alternative<request_states::next>(execute_random_play(origin, bot_info.settings.response_rules, play_cards))) {
                    return request_states::next{};
                }
            }
        } else if (m_playing && m_playing->is_bot()) {
            playable_cards_list play_cards = generate_playable_cards_list(m_playing);
            return execute_random_play(m_playing, bot_info.settings.in_play_rules, play_cards);
        }
        return request_states::done{};
    }

    
}