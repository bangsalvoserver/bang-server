#ifndef __TARGET_TYPE_SELECT_CUBES_H__
#define __TARGET_TYPE_SELECT_CUBES_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_select_cubes : targeting_base {
        using targeting_base::targeting_base;
        
        using value_type = card_list;
        
        std::generator<card_list> possible_targets(const effect_context &ctx);
        card_list random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(const effect_context &ctx, const card_list &target);
        void add_context(effect_context &ctx, const card_list &target);
        void on_play(const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(select_cubes, targeting_select_cubes)
}

#endif