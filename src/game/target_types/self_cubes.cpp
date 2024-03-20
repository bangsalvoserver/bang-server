#include "game/play_verify.h"

#include "cards/effect_enums.h"

namespace banggame {

    using visit_cubes = play_visitor<target_type::self_cubes>;

    template<> game_string visit_cubes::get_error(const effect_context &ctx) {
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

    template<> void visit_cubes::add_context(effect_context &ctx) {
        for (int i=0; i < effect.target_value; ++i) {
            ctx.selected_cubes.push_back(origin_card);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx) {}

}