#include "select_cubes_player.h"

#include "player.h"
#include "select_cubes_optional.h"

namespace banggame {

    targeting_select_cubes_player::value_type targeting_select_cubes_player::random_target(const effect_context &ctx) {
        auto cubes = targeting_select_cubes_optional{*this}.random_target(ctx);
        auto target = targeting_player{*this}.random_target(ctx);
        return { cubes, target };
    }

    game_string targeting_select_cubes_player::get_error(const effect_context &ctx, const value_type &target) {
        MAYBE_RETURN(targeting_select_cubes_optional{*this}.get_error(ctx, target.cubes));
        MAYBE_RETURN(targeting_player{*this}.get_error(ctx, target.player));
        return {};
    }

    prompt_string targeting_select_cubes_player::on_prompt(const effect_context &ctx, const value_type &target) {
        MAYBE_RETURN(targeting_select_cubes_optional{*this}.on_prompt(ctx, target.cubes));
        MAYBE_RETURN(targeting_player{*this}.on_prompt(ctx, target.player));
        return {};
    }

    void targeting_select_cubes_player::add_context(effect_context &ctx, const value_type &target) {
        targeting_select_cubes_optional{*this}.add_context(ctx, target.cubes);
        targeting_player{*this}.add_context(ctx, target.player);
    }

    void targeting_select_cubes_player::on_play(const effect_context &ctx, const value_type &target) {
        targeting_select_cubes_optional{*this}.on_play(ctx, target.cubes);
        targeting_player{*this}.on_play(ctx, target.player);
    }

}