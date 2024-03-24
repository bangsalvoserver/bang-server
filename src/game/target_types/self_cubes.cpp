#include "game/play_verify.h"

#include "cards/effect_enums.h"

namespace banggame {

    using visit_cubes = play_visitor<target_type::self_cubes>;

    template<> game_string visit_cubes::get_error(const effect_context &ctx) {
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx) {
        for (int i=0; i < effect.target_value; ++i) {
            effect.add_context(origin_card, origin, origin_card, ctx);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}