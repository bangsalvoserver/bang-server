#include "extra_card.h"

#include "card.h"

#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    std::generator<card_ptr> targeting_extra_card::possible_targets(const effect_context &ctx) {
        if (ctx.repeat_card) {
            co_yield nullptr;
        } else {
            for (card_ptr target : get_all_card_targets(origin, origin_card, effect, ctx)) {
                co_yield target;
            }
        }
    }

    card_ptr targeting_extra_card::random_target(const effect_context &ctx) {
        if (ctx.repeat_card) {
            return nullptr;
        } else {
            return targeting_card{*this}.random_target(ctx);
        }
    }

    game_string targeting_extra_card::get_error(const effect_context &ctx, card_ptr target) {
        if (!target) {
            if (ctx.repeat_card) {
                return {};
            } else {
                return "ERROR_TARGET_SET_NOT_EMPTY";
            }
        } else {
            if (ctx.repeat_card) {
                return "ERROR_TARGET_SET_EMPTY";
            } else {
                return targeting_card{*this}.get_error(ctx, target);
            }
        }
    }

    prompt_string targeting_extra_card::on_prompt(const effect_context &ctx, card_ptr target) {
        if (target) {
            return targeting_card{*this}.on_prompt(ctx, target);
        } else {
            return {};
        }
    }

    void targeting_extra_card::add_context(effect_context &ctx, card_ptr target) {
        if (target) {
            targeting_card{*this}.add_context(ctx, target);
        }
    }

    void targeting_extra_card::on_play(const effect_context &ctx, card_ptr target) {
        if (target) {
            return targeting_card{*this}.on_play(ctx, target);
        }
    }

}