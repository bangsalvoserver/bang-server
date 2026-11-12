#include "max_cards.h"

namespace banggame {

    bool targeting_max_cards::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        if (confirmable) {
            return contains_at_least(target_card.possible_targets(origin_card, origin, effect, ctx), 1);
        } else {
            auto targets = target_card.possible_targets(origin_card, origin, {}, ctx);
            size_t required = 0, valid = 0;
            for (card_ptr target : targets) {
                if (required < ncards) ++required;
                if (!target_card.get_error(origin_card, origin, effect, ctx, target)) ++valid;
                if (required == ncards && valid >= required) return true;
            }
            return required > 0 && valid >= required;
        }
    }

    std::pair<card_list, int> targeting_max_cards::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        card_list candidates = rn::to<card_list>(target_card.possible_targets(origin_card, origin, effect, ctx));
        auto evaluate_fun = [&](card_ptr target) {
            if (effect.on_prompt(origin_card, origin, target, {}, ctx)) return -2;
            return effect.evaluate(origin_card, origin, target, ctx);
        };

        size_t max_count = ncards == 0 ? candidates.size() : std::min<size_t>(ncards, candidates.size());
        size_t min_count = confirmable ? 1 : std::min<size_t>(std::max(ncards, 1), candidates.size());
        if (!confirmable) {
            max_count = min_count;
        }

        std::optional<std::pair<card_list, int>> best;
        size_t ties = 0;
        for (size_t count = min_count; count <= max_count; ++count) {
            auto candidate = random_max_elements(candidates, count, evaluate_fun, origin->m_game->bot_rng);
            if (!best || candidate.second > best->second) {
                best = std::move(candidate);
                ties = 1;
            } else if (candidate.second == best->second) {
                ++ties;
                std::uniform_int_distribution<size_t> dist(0, ties - 1);
                if (dist(origin->m_game->bot_rng) == 0) {
                    best = std::move(candidate);
                }
            }
        }
        return std::move(*best);
    }

    game_string targeting_max_cards::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) const {
        if (confirmable) {
            if (targets.empty() || ncards != 0 && targets.size() > ncards) {
                return "ERROR_INVALID_TARGETS";
            }
        } else {
            int count = 0;
            for (card_ptr c : target_card.possible_targets(origin_card, origin, {}, ctx)) {
                ++count;
                if (count >= ncards) break;
            }
            if (targets.empty() || targets.size() != count) {
                return "ERROR_INVALID_TARGETS";
            }
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(target_card.get_error(origin_card, origin, effect, ctx, c));
        }
        return {};
    }

}
