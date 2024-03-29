#include "game/play_verify.h"

namespace banggame {

    using visit_cubes = play_visitor<target_type::self_cubes>;
    
    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return origin_card->num_cubes >= effect.target_value;
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx) {
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx) {
        for (int i=0; i < effect.target_value; ++i) {
            effect.type->add_context_card(effect.effect_value, origin_card, origin, origin_card, ctx);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx) {
        effect.type->on_play(effect.effect_value, origin_card, origin, {}, ctx);
    }

}