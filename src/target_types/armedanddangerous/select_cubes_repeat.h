#ifndef __TARGET_TYPE_SELECT_CUBES_REPEAT_H__
#define __TARGET_TYPE_SELECT_CUBES_REPEAT_H__

#include "cards/card_effect.h"

#include "select_cubes.h"

namespace banggame {

    struct targeting_select_cubes_repeat : targeting_select_cubes {
        using targeting_select_cubes::targeting_select_cubes;
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return true;
        }

        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(select_cubes_repeat, targeting_select_cubes_repeat)
}

#endif