#include "game/play_verify.h"

namespace banggame {

    using visit_none = play_visitor<target_types::none>;

    template<> game_string visit_none::get_error(const effect_context &ctx, target_types::none) {
        return effect.get_error(origin_card, origin, ctx);
    }

    template<> std::generator<target_types::none> visit_none::possible_targets(const effect_context &ctx) {
        if (!get_error(ctx, {})) {
            co_yield {};
        }
    }

    template<> target_types::none visit_none::random_target(const effect_context &ctx) {
        return {};
    }

    template<> prompt_string visit_none::prompt(const effect_context &ctx, target_types::none) {
        return effect.on_prompt(origin_card, origin, ctx);
    }

    template<> void visit_none::add_context(effect_context &ctx, target_types::none) {
        effect.add_context(origin_card, origin, ctx);
    }

    template<> void visit_none::play(const effect_context &ctx, target_types::none) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}