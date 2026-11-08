#include "game.h"
#include "filters.h"
#include "game_options.h"
#include "play_verify.h"
#include "possible_to_play.h"
#include "request_timer.h"

#include "effects/base/equip.h"

#include "net/bot_info.h"
#include "net/logging.h"

namespace banggame {

    struct scored_action {
        game_action action;
        int score;
    };

    static scored_action generate_random_play(player_ptr origin, const playable_card_info &args) {
        scored_action ret {
            .action {
                .card = args.card,
                .effect_list = args.effect_list
            },
            .score = 0
        };
        effect_context ctx;
        
        for (const auto &[mod_card, mod_response] : args.modifiers) {
            auto &targets = ret.action.modifiers.emplace_back(mod_card, mod_response).targets;

            mod_card->get_modifier(mod_response).add_context(mod_card, origin, ctx);
            for (const effect_holder &holder : mod_card->get_effect_list(mod_response)) {
                auto value = holder.random_target(mod_card, origin, ctx);
                holder.add_context(mod_card, origin, value.target, ctx);
                targets.emplace_back(std::move(value.target));
                ret.score += value.score;
            }
            if (const mth_holder &mth = mod_card->get_mth(mod_response)) {
                ret.score += mth.evaluate(mod_card, origin, targets, ctx);
            }
        }

        const effect_list &effects = args.card->get_effect_list(args.effect_list);
        for (const effect_holder &holder : effects) {
            auto value = holder.random_target(args.card, origin, ctx);
            holder.add_context(args.card, origin, value.target, ctx);
            ret.action.targets.emplace_back(std::move(value.target));
            ret.score += value.score;
        }
        if (args.effect_list == effect_list_type::equip_effects) {
            if (!ctx.contains<contexts::equip_target>()) {
                ret.score += effect_equip_on{}.evaluate(args.card, origin, origin, ctx);
            }
        } else {
            if (const mth_holder &mth = args.card->get_mth(args.effect_list)) {
                ret.score += mth.evaluate(args.card, origin, ret.action.targets, ctx);
            }
        }

        return ret;
    }

    // A candidate action. The "greatest" candidate is the best one: highest score, then lowest
    // rule priority index, then lowest random tiebreak. The tiebreak is drawn once per candidate
    // so that the ordering is a strict total order and equally good actions are chosen uniformly.
    struct candidate {
        int score;
        int priority;
        uint32_t tiebreak;
        size_t index; // index in the plays vector, or no_play for "do nothing"

        static constexpr size_t no_play = static_cast<size_t>(-1);

        auto operator<=>(const candidate &rhs) const {
            if (auto cmp = score <=> rhs.score; cmp != 0) return cmp;
            if (auto cmp = rhs.priority <=> priority; cmp != 0) return cmp;
            return rhs.tiebreak <=> tiebreak;
        }
    };

    // Rules are used as tiebreakers: the priority is the index of the first matching rule,
    // actions that match no rule come last.
    static int get_rule_priority(const bot_rule_list &rules, card_node node) {
        int priority = 0;
        for (const bot_rule &rule : rules) {
            if (rule(node)) break;
            ++priority;
        }
        return priority;
    }

    static request_state execute_best_play(player_ptr origin, const bot_rule_list &bot_rules, const playable_cards_list &play_cards) {
        auto &rng = origin->m_game->bot_rng;

        auto timer = origin->m_game->top_request<request_timer>();
        bool add_empty = timer && timer->enabled();

        // every action is generated and scored exactly once
        std::vector<scored_action> plays;
        std::vector<candidate> candidates;
        plays.reserve(play_cards.size());
        candidates.reserve(play_cards.size() + add_empty);

        for (const playable_card_info &info : play_cards) {
            auto &play = plays.emplace_back(generate_random_play(origin, info));
            candidates.push_back(candidate {
                .score = play.score,
                .priority = get_rule_priority(bot_rules, &info),
                .tiebreak = static_cast<uint32_t>(rng()),
                .index = plays.size() - 1
            });
        }

        // doing nothing is worth 0: it is preferred to every harmful action
        if (add_empty) {
            candidates.push_back(candidate {
                .score = 0,
                .priority = get_rule_priority(bot_rules, nullptr),
                .tiebreak = static_cast<uint32_t>(rng()),
                .index = candidate::no_play
            });
        }

        // Attempts are made in rounds: every candidate is tried once per round, and the ones that fail
        // are regenerated and wait for the next round, so that a failing card does not starve the others.
        // A negative score, a prompt and an error all count as a failed attempt.
        for (int round = 0; round <= bot_info.settings.max_retries; ++round) {
            // from this round onward harmful and prompting actions are accepted
            const bool bypass = round >= bot_info.settings.bypass_retries;

            // building a heap is O(n). pop_heap moves the best candidate to the end of the heap range,
            // so tried candidates stay in the vector, outside of the heap, until the next round
            std::make_heap(candidates.begin(), candidates.end());

            for (auto last = candidates.end(); last != candidates.begin(); --last) {
                std::pop_heap(candidates.begin(), last);
                candidate &best = *std::prev(last);

                if (best.index == candidate::no_play) {
                    return request_states::done{};
                }

                const playable_card_info &info = play_cards[best.index];
                scored_action &play = plays[best.index];

                if (best.score >= 0 || bypass) {
                    play.action.bypass_prompt = bypass;
                    auto result = verify_and_play(origin, play.action);

                    if (std::holds_alternative<play_verify_results::ok>(result)) {
                        return request_states::next{};
                    } else if (auto *prompt = std::get_if<play_verify_results::prompt>(&result)) {
                        logging::trace("BOT PROMPT: message={}, score={}, round={}", std::string_view{prompt->message.message.format_str}, best.score, round);
                    } else if (auto *error = std::get_if<play_verify_results::error>(&result)) {
                        logging::trace("BOT ERROR: message={}, score={}, round={}", std::string_view{error->message.format_str}, best.score, round);
                    }
                }

                // targets are chosen among equally scored elements at random, so a new attempt may differ
                play = generate_random_play(origin, info);
                best.score = play.score;
                best.tiebreak = static_cast<uint32_t>(rng());
            }
        }

        // softlock
        logging::warn("BOT ERROR: could not find card in execute_best_play()");

        return request_states::done{};
    }

    request_state game::request_bot_play(player_ptr origin, bool instant) {
        if (origin) {
            if (pending_requests()) {
                if (!instant && m_options.bot_play_timer > game_duration{0}) {
                    if (!get_all_playable_cards(origin, effect_list_type::responses).empty()) {
                        return request_states::bot_play{ origin, get_total_update_time() + std::chrono::duration_cast<ticks>(m_options.bot_play_timer) };
                    }
                } else {       
                    playable_cards_list play_cards = generate_playable_cards_list(origin, effect_list_type::responses);
                    if (!play_cards.empty() && std::holds_alternative<request_states::next>(execute_best_play(origin, bot_info.settings.response_rules, play_cards))) {
                        return request_states::next{};
                    }
                }
            } else if (origin == origin->m_game->m_playing) {
                if (!instant && m_options.bot_play_timer > game_duration{0}) {
                    if (!get_all_playable_cards(origin, effect_list_type::effects).empty()) {
                        return request_states::bot_play{ origin, get_total_update_time() + std::chrono::duration_cast<ticks>(m_options.bot_play_timer) };
                    }
                } else {       
                    playable_cards_list play_cards = generate_playable_cards_list(origin, effect_list_type::effects);
                    if (!play_cards.empty() && std::holds_alternative<request_states::next>(execute_best_play(origin, bot_info.settings.in_play_rules, play_cards))) {
                        return request_states::next{};
                    }
                }
            }
        } else if (auto all_bots = m_players | rv::filter(&player::is_bot)) {
            if (!instant && m_options.bot_play_timer > game_duration{0}) {
                return request_states::bot_play{ nullptr, get_total_update_time() + std::chrono::duration_cast<ticks>(m_options.bot_play_timer) };
            }

            if (pending_requests()) {
                for (player_ptr p : all_bots) {
                    playable_cards_list play_cards = generate_playable_cards_list(p, effect_list_type::responses);
                    if (!play_cards.empty() && std::holds_alternative<request_states::next>(execute_best_play(p, bot_info.settings.response_rules, play_cards))) {
                        return request_states::next{};
                    }
                }
            } else if (m_playing && m_playing->is_bot()) {
                playable_cards_list play_cards = generate_playable_cards_list(m_playing, effect_list_type::effects);
                if (!play_cards.empty() && std::holds_alternative<request_states::next>(execute_best_play(m_playing, bot_info.settings.in_play_rules, play_cards))) {
                    return request_states::next{};
                }
            }
        }
        return request_states::done{};
    }

    
}
