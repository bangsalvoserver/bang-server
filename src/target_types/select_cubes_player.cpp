#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<target_types::select_cubes_player>;

    template<> std::generator<target_types::select_cubes_player> visit_cubes::possible_targets(const effect_context &ctx) {
        for (player_ptr target : get_all_player_targets(origin, origin_card, effect, ctx)) {
            co_yield { .player = target };
        }
    }

    template<> target_types::select_cubes_player visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = defer<target_types::select_cubes_optional>().random_target(ctx);
        auto target = defer<target_types::player>().random_target(ctx);
        return { cubes, target };
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const target_types::select_cubes_player &target) {
        MAYBE_RETURN(defer<target_types::select_cubes_optional>().get_error(ctx, target.cubes));
        MAYBE_RETURN(defer<target_types::player>().get_error(ctx, target.player));
        return {};
    }

    template<> prompt_string visit_cubes::prompt(const effect_context &ctx, const target_types::select_cubes_player &target) {
        MAYBE_RETURN(defer<target_types::select_cubes_optional>().prompt(ctx, target.cubes));
        MAYBE_RETURN(defer<target_types::player>().prompt(ctx, target.player));
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const target_types::select_cubes_player &target) {
        defer<target_types::select_cubes_optional>().add_context(ctx, target.cubes);
        defer<target_types::player>().add_context(ctx, target.player);
    }

    template<> void visit_cubes::play(const effect_context &ctx, const target_types::select_cubes_player &target) {
        defer<target_types::select_cubes_optional>().play(ctx, target.cubes);
        defer<target_types::player>().play(ctx, target.player);
    }

}