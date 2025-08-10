#ifndef __TARGET_TYPE_SELECT_CUBES_OPTIONAL_H__
#define __TARGET_TYPE_SELECT_CUBES_OPTIONAL_H__

#include "select_cubes.h"

#include "game/possible_to_play.h"

namespace banggame {

    struct targeting_select_cubes_optional : targeting_select_cubes {
        using targeting_select_cubes::targeting_select_cubes;
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return true;
        }

        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            if (count_cubes(origin) >= ncubes) {
                return targeting_select_cubes::random_target(origin_card, origin, effect, ctx);
            }
            return {};
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) {
            if (!target.empty()) {
                return targeting_select_cubes::get_error(origin_card, origin, effect, ctx, target);
            }
            return {};
        }
    };

    DEFINE_TARGETING(select_cubes_optional, targeting_select_cubes_optional)
}

#endif