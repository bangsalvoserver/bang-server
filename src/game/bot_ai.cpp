#include "game.h"
#include "filters.h"
#include "game_options.h"
#include "play_verify.h"
#include "possible_to_play.h"

#include "net/bot_info.h"
#include "net/logging.h"

namespace banggame {

    static game_action generate_random_play(player_ptr origin, const playable_card_info &args, bool is_response) {
        game_action ret { .card = args.card };
        effect_context ctx{};
        
        for (card_ptr mod_card : args.modifiers) {
            auto &targets = ret.modifiers.emplace_back(mod_card).targets;

            mod_card->get_modifier(is_response).add_context(mod_card, origin, ctx);
            for (const effect_holder &holder : mod_card->get_effect_list(is_response)) {
                const auto &target = targets.emplace_back(play_dispatch::random_target(origin, mod_card, holder, ctx));
                play_dispatch::add_context(origin, mod_card, holder, ctx, target);
            }
        }

        if (args.card->is_equip_card()) {
            if (!args.card->self_equippable()) {
                ret.targets.emplace_back(TAG(player),
                    random_element(get_all_equip_targets(origin, args.card, ctx), origin->m_game->bot_rng));
            }
        } else {
            for (const effect_holder &holder : args.card->get_effect_list(is_response)) {
                const auto &target = ret.targets.emplace_back(play_dispatch::random_target(origin, args.card, holder, ctx));
                play_dispatch::add_context(origin, args.card, holder, ctx, target);
            }
        }

        return ret;
    }

    using node_set_t = std::multiset<card_node>;
    
    static card_node get_selected_node(player_ptr origin, bool is_response, const node_set_t &node_set) {
        auto &rules = is_response ? bot_info.settings.response_rules : bot_info.settings.in_play_rules;
        for (const bot_rule &rule : rules) {
            if (auto filter = rv::filter(node_set, rule)) {
                return random_element(filter, origin->m_game->bot_rng);
            }
        }
        return random_element(node_set, origin->m_game->bot_rng);
    }

    static request_state execute_random_play(player_ptr origin, bool is_response, std::optional<timer_id_t> timer_id, const playable_cards_list &play_cards) {
        for (int i=0; i < bot_info.settings.max_random_tries; ++i) {
            auto node_set = play_cards
                | rv::for_each([&](const playable_card_info &info) {
                    return rv::repeat_n(&info, bot_info.settings.repeat_card_nodes);
                })
                | rn::to<node_set_t>;
            
            if (timer_id) {
                node_set.insert(nullptr);
            }
            
            while (!node_set.empty()) {
                auto selected_node = get_selected_node(origin, is_response, node_set);
                node_set.erase(node_set.find(selected_node));

                if (!selected_node) {
                    return TAG(done);
                }

                try {
                    auto args = generate_random_play(origin, *selected_node, is_response);
                    args.bypass_prompt =
                        (i >= bot_info.settings.bypass_empty_index && node_set.empty())
                        || i >= bot_info.settings.bypass_unconditional_index;
                    args.timer_id = timer_id;

                    auto result = verify_and_play(origin, args);

                    if (utils::visit_tagged(overloaded{
                        [](TAG_T(ok)) {
                            return true;
                        },
                        [&](TAG_T(prompt), const prompt_string &prompt) {
                            logging::trace("BOT PROMPT: message={}, i={}", std::string_view{prompt.message.format_str}, i);
                            return false;
                        },
                        [&](TAG_T(error), const game_string &error) {
                            logging::trace("BOT ERROR: message={}, i={}", std::string_view{error.format_str}, i);
                            return false;
                        }
                    }, result)) {
                        return TAG(next);
                    }
                } catch (const random_element_error &) {
                    // ignore
                }
            }
        }

        // softlock
        logging::warn("BOT ERROR: could not find card in execute_random_play()");

        return TAG(done);
    }

    request_state game::request_bot_play(bool instant) {
        if (m_options.num_bots == 0) {
            return TAG(done);
        } else if (!instant && m_options.bot_play_timer > game_duration{0}) {
            return { TAG(bot_play), get_total_update_time() + std::chrono::duration_cast<ticks>(m_options.bot_play_timer) };
        }

        if (pending_requests()) {
            for (player_ptr origin : m_players | rv::filter(&player::is_bot)) {
                playable_cards_list play_cards = generate_playable_cards_list(origin, true);
                
                if (!play_cards.empty()) {
                    std::optional<timer_id_t> timer_id;
                    if (request_timer *timer = origin->m_game->top_request()->timer()) {
                        timer_id = timer->get_timer_id();
                    }

                    if (holds_alternative<"next">(execute_random_play(origin, true, timer_id, play_cards))) {
                        return TAG(next);
                    }
                }
            }
        } else if (m_playing && m_playing->is_bot()) {
            playable_cards_list play_cards = generate_playable_cards_list(m_playing);
            return execute_random_play(m_playing, false, std::nullopt, play_cards);
        }
        return TAG(done);
    }

    
}