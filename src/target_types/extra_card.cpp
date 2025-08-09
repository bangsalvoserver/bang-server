#include "extra_card.h"

#include "card.h"

#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    std::generator<card_ptr> targeting_extra_card::possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        if (ctx.repeat_card) {
            co_yield nullptr;
        } else {
            for (card_ptr target : targeting_card::possible_targets(origin_card, origin, effect, ctx)) {
                co_yield target;
            }
        }
    }

    card_ptr targeting_extra_card::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        if (ctx.repeat_card) {
            return nullptr;
        } else {
            return targeting_card::random_target(origin_card, origin, effect, ctx);
        }
    }

    game_string targeting_extra_card::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
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
                return targeting_card::get_error(origin_card, origin, effect, ctx, target);
            }
        }
    }

    prompt_string targeting_extra_card::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
        if (target) {
            return targeting_card::on_prompt(origin_card, origin, effect, ctx, target);
        } else {
            return {};
        }
    }

    void targeting_extra_card::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, card_ptr target) {
        if (target) {
            targeting_card::add_context(origin_card, origin, effect, ctx, target);
        }
    }

    void targeting_extra_card::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
        if (target) {
            return targeting_card::on_play(origin_card, origin, effect, ctx, target);
        }
    }

}