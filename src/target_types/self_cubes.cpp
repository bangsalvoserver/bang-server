#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<target_types::self_cubes>;

    template<> game_string visit_cubes::get_error(const effect_context &ctx, target_types::self_cubes) {
        if (origin_card->num_cubes() < effect.target_value) {
            return "ERROR_NOT_ENOUGH_CUBES";
        }
        return {};
    }
    
    template<> std::generator<target_types::self_cubes> visit_cubes::possible_targets(const effect_context &ctx) {
        if (!get_error(ctx, {})) {
            co_yield {};
        }
    }

    template<> target_types::self_cubes visit_cubes::random_target(const effect_context &ctx) {
        return {};
    }

    template<> prompt_string visit_cubes::prompt(const effect_context &ctx, target_types::self_cubes) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx, target_types::self_cubes) {
        ctx.selected_cubes.insert(origin_card,
            card_list{static_cast<size_t>(effect.target_value), origin_card},
            effect.target_value);
    }

    template<> void visit_cubes::play(const effect_context &ctx, target_types::self_cubes) {
        origin_card->move_cubes(nullptr, effect.target_value);
    }

}