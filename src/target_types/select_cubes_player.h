#ifndef __TARGET_TYPE_SELECT_CUBES_PLAYER_H__
#define __TARGET_TYPE_SELECT_CUBES_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_select_cubes_player : targeting_base {
        using targeting_base::targeting_base;

        struct value_type {
            struct transparent{};
            card_list cubes;
            player_ptr player;
        };
        
        std::generator<value_type> possible_targets(const effect_context &ctx);
        value_type random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const value_type &target);
        prompt_string on_prompt(const effect_context &ctx, const value_type &target);
        void add_context(effect_context &ctx, const value_type &target);
        void on_play(const effect_context &ctx, const value_type &target);
    };

    DEFINE_TARGETING(select_cubes_player, targeting_select_cubes_player)
}

#endif