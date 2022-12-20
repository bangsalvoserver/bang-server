#include "game/play_verify.h"

#include "cards/effect_enums.h"

namespace banggame {

    using visit_cubes = play_visitor<target_type::self_cubes>;

    template<> game_string visit_cubes::get_error(const effect_context &ctx) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        return {};
    }

    template<> duplicate_set visit_cubes::duplicates() {
        return {.cubes{
            {origin_card, effect.target_value}
        }};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx) {
        return {};
    }

    template<> void visit_cubes::play(const effect_context &ctx) {}

}