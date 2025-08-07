#ifndef __TARGET_TYPE_PLAYER_H__
#define __TARGET_TYPE_PLAYER_H__

#include "cards/card_effect.h"

#include "game/possible_to_play.h"

namespace banggame {

    struct targeting_player : targeting_base {
        using targeting_base::targeting_base;

        using value_type = player_ptr;
        
        auto possible_targets(const effect_context &ctx) {
            return get_all_player_targets(origin, origin_card, effect, ctx);
        }

        player_ptr random_target(const effect_context &ctx) {
            return random_element(possible_targets(ctx), origin->m_game->bot_rng);
        }

        game_string get_error(const effect_context &ctx, player_ptr target);
        prompt_string on_prompt(const effect_context &ctx, player_ptr target);
        void add_context(effect_context &ctx, player_ptr target);
        void on_play(const effect_context &ctx, player_ptr target);
    };

    DEFINE_TARGETING(player, targeting_player)
}

#endif