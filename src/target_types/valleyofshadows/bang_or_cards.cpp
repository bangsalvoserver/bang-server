#include "bang_or_cards.h"

namespace banggame {

    bool targeting_bang_or_cards::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        return contains_at_least(target_card.possible_targets(origin_card, origin, effect, ctx), 1);
    }

    std::pair<card_list, int> targeting_bang_or_cards::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        auto targets = target_card.possible_targets(origin_card, origin, effect, ctx);
        auto bang_targets = targets | rv::filter([&](card_ptr target_card) { return target_card->is_bang_card(origin); });

        auto evaluate_fun = [&](card_ptr target_card) {
            if (effect.on_prompt(origin_card, origin, target_card, {}, ctx)) return -2;
            return effect.evaluate(origin_card, origin, target_card, ctx);
        };

        if (rn::begin(bang_targets) != rn::end(bang_targets)) {
            return random_max_elements(bang_targets, 1, evaluate_fun, origin->m_game->bot_rng);
        }
        return random_max_elements(targets, ncards, evaluate_fun, origin->m_game->bot_rng);
    }

    game_string targeting_bang_or_cards::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) const {
        if (targets.size() == 1) {
            if (!targets.front()->is_bang_card(origin)) {
                return "ERROR_TARGET_NOT_BANG";
            }
        } else if (targets.size() != ncards) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(target_card.get_error(origin_card, origin, effect, ctx, c));
        }
        return {};
    }

}
