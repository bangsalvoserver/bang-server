#include "bang_or_cards.h"

#include "card.h"

namespace banggame {

    bool targeting_bang_or_cards::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        return bool(target_card.possible_targets(origin_card, origin, effect, ctx));
    }

    card_list targeting_bang_or_cards::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto targets = target_card.possible_targets(origin_card, origin, effect, ctx);
        if (auto bang_targets = targets | rv::filter([&](card_ptr target_card) {
            return target_card->is_bang_card(origin);
        })) {
            return std::vector{random_element(bang_targets, origin->m_game->bot_rng)};
        }
        return targets
            | rv::sample(ncards, origin->m_game->bot_rng)
            | rn::to_vector;
    }

    game_string targeting_bang_or_cards::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
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