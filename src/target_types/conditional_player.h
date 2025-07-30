#ifndef __TARGET_TYPE_CONDITIONAL_PLAYER_H__
#define __TARGET_TYPE_CONDITIONAL_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_conditional_player : targeting_base {
        using targeting_base::targeting_base;
        
        using value_type = nullable_player;
        
        std::generator<player_ptr> possible_targets(const effect_context &ctx);
        player_ptr random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, player_ptr target);
        prompt_string on_prompt(const effect_context &ctx, player_ptr target);
        void add_context(effect_context &ctx, player_ptr target);
        void on_play(const effect_context &ctx, player_ptr target);
    };

    DEFINE_TARGETING(conditional_player, targeting_conditional_player)
}

#endif