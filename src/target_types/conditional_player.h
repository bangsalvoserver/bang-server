#ifndef __TARGET_TYPE_CONDITIONAL_PLAYER_H__
#define __TARGET_TYPE_CONDITIONAL_PLAYER_H__

#include "player.h"

namespace banggame {

    struct targeting_conditional_player : targeting_player {
        using targeting_player::targeting_player;

        using value_type = nullable_player;
        
        std::generator<player_ptr> possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            if (auto targets = targeting_player::possible_targets(origin_card, origin, effect, ctx)) {
                for (player_ptr target : targets) {
                    co_yield target;
                }
            } else {
                co_yield nullptr;
            }
        }

        player_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            if (auto targets = targeting_player::possible_targets(origin_card, origin, effect, ctx)) {
                return random_element(targets, origin->m_game->bot_rng);
            } else {
                return nullptr;
            }
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) {
            if (target) {
                return targeting_player::get_error(origin_card, origin, effect, ctx, target);
            } else if (bool(targeting_player::possible_targets(origin_card, origin, effect, ctx))) {
                return "ERROR_TARGET_SET_NOT_EMPTY";
            } else {
                return {};
            }
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) {
            if (target) {
                return targeting_player::on_prompt(origin_card, origin, effect, ctx, target);
            } else {
                return {"PROMPT_CARD_NO_TARGET", origin_card};
            }
        }

        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_ptr target) {
            if (target) {
                targeting_player::add_context(origin_card, origin, effect, ctx, target);
            }
        }

        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) {
            if (target) {
                targeting_player::on_play(origin_card, origin, effect, ctx, target);
            }
        }
    };

    DEFINE_TARGETING(conditional_player, targeting_conditional_player)
}

#endif