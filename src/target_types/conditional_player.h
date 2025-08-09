#ifndef __TARGET_TYPE_CONDITIONAL_PLAYER_H__
#define __TARGET_TYPE_CONDITIONAL_PLAYER_H__

#include "player.h"

namespace banggame {

    struct targeting_conditional_player : targeting_player {
        using targeting_player::targeting_player;

        using value_type = nullable_player;
        
        std::generator<player_ptr> possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        player_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target);
    };

    DEFINE_TARGETING(conditional_player, targeting_conditional_player)
}

#endif