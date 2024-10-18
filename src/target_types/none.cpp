#include "game/play_verify.h"

namespace banggame {

    using visit_none = play_visitor<"none">;

    template<> bool visit_none::possible(const effect_context &ctx) {
        return !effect.get_error(origin_card, origin, ctx);
    }

    template<> game_string visit_none::get_error(const effect_context &ctx) {
        return effect.get_error(origin_card, origin, ctx);
    }

    template<> prompt_string visit_none::prompt(const effect_context &ctx) {
        return effect.on_prompt(origin_card, origin, ctx);
    }

    template<> void visit_none::add_context(effect_context &ctx) {
        effect.add_context(origin_card, origin, ctx);
    }

    template<> void visit_none::play(const effect_context &ctx) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}