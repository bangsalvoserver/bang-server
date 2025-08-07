#ifndef __TARGET_TYPE_SELECT_CUBES_OPTIONAL_H__
#define __TARGET_TYPE_SELECT_CUBES_OPTIONAL_H__

#include "cards/card_effect.h"

#include "select_cubes.h"

namespace banggame {

    struct targeting_select_cubes_optional : targeting_select_cubes {
        using targeting_select_cubes::targeting_select_cubes;
        
        bool is_possible(const effect_context &ctx) {
            return true;
        }

        card_list random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(select_cubes_optional, targeting_select_cubes_optional)
}

#endif