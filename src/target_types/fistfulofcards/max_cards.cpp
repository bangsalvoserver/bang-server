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

    scored_target<card_list> targeting_max_cards::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        auto targets = target_card.possible_targets(origin_card, origin, effect, ctx);

        auto evaluate_fun = [&](card_ptr target) {
            if (effect.on_prompt(origin_card, origin, target, {}, ctx)) return -2;
            return effect.evaluate(origin_card, origin, target, ctx);
        };
        
        if (ncards == 0) {
            size_t count_targets = rn::count_if(targets, [&](card_ptr c) { return evaluate_fun(c) >= 0; });
            return random_max_elements(targets, count_targets == 0 ? 1 : count_targets, evaluate_fun, origin->m_game->bot_rng);
        } else {
            return random_max_elements(targets, ncards, evaluate_fun, origin->m_game->bot_rng);
        }
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
