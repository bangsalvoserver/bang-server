#ifndef __TARGET_TYPE_EXTRA_CARD_H__
#define __TARGET_TYPE_EXTRA_CARD_H__

#include "target_types/base/card.h"

namespace banggame {

    struct targeting_extra_card : targeting_card {
        using targeting_card::targeting_card;

        using value_type = nullable_card;

        std::generator<card_ptr> possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            if (ctx.contains<contexts::repeat_card>()) {
                co_yield nullptr;
            } else {
                for (card_ptr target : targeting_card::possible_targets(origin_card, origin, effect, ctx)) {
                    co_yield target;
                }
            }
        }

        card_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            if (ctx.contains<contexts::repeat_card>()) {
                return nullptr;
            } else {
                return random_element(targeting_card::possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng);
            }
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
            if (!target) {
                if (ctx.contains<contexts::repeat_card>()) {
                    return {};
                } else {
                    return "ERROR_TARGET_SET_NOT_EMPTY";
                }
            } else {
                if (ctx.contains<contexts::repeat_card>()) {
                    return "ERROR_TARGET_SET_EMPTY";
                } else {
                    return targeting_card::get_error(origin_card, origin, effect, ctx, target);
                }
            }
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
            if (target) {
                return targeting_card::on_prompt(origin_card, origin, effect, ctx, target);
            } else {
                return {};
            }
        }

        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, card_ptr target) {
            if (target) {
                targeting_card::add_context(origin_card, origin, effect, ctx, target);
            }
        }

        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
            if (target) {
                return targeting_card::on_play(origin_card, origin, effect, ctx, target);
            }
        }
    };

    DEFINE_TARGETING(extra_card, targeting_extra_card)
}

#endif