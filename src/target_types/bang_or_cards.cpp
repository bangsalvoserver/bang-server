#include "bang_or_cards.h"

#include "card.h"

#include "game/possible_to_play.h"

namespace banggame {

    std::generator<card_list> targeting_bang_or_cards::possible_targets(const effect_context &ctx) {
        if (get_all_card_targets(origin, origin_card, effect, ctx)) {
            co_yield {};
        }
    }

    card_list targeting_bang_or_cards::random_target(const effect_context &ctx) {
        auto targets = get_all_card_targets(origin, origin_card, effect, ctx);
        if (auto bang_targets = targets | rv::filter([&](card_ptr target_card) {
            return target_card->is_bang_card(origin);
        })) {
            return std::vector{random_element(bang_targets, origin->m_game->bot_rng)};
        }
        return targets
            | rv::sample(effect.target_value, origin->m_game->bot_rng)
            | rn::to_vector;
    }

    game_string targeting_bang_or_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.size() == 1) {
            if (!targets.front()->is_bang_card(origin)) {
                return "ERROR_TARGET_NOT_BANG";
            }
        } else if (targets.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : targets) {
            MAYBE_RETURN(targeting_card{*this}.get_error(ctx, c));
        }
        return {};
    }

}