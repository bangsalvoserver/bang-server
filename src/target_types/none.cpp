#include "game/play_verify.h"

namespace banggame {

    using visit_none = play_visitor<target_type::none>;

    template<> bool visit_none::possible(const effect_context &ctx) {
        return !effect.type->get_error(effect.effect_value, origin_card, origin, ctx);
    }

    template<> game_string visit_none::get_error(const effect_context &ctx) {
        return effect.type->get_error(effect.effect_value, origin_card, origin, ctx);
    }

    template<> game_string visit_none::prompt(const effect_context &ctx) {
        return effect.type->on_prompt(effect.effect_value, origin_card, origin, ctx);
    }

    template<> void visit_none::add_context(effect_context &ctx) {
        effect.type->add_context(effect.effect_value, origin_card, origin, ctx);
    }

    template<> void visit_none::play(const effect_context &ctx) {
        effect.type->on_play(effect.effect_value, origin_card, origin, {}, ctx);
    }

}