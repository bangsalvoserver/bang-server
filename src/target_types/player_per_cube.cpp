#include "game/possible_to_play.h"

namespace banggame {

    using visit_players = play_visitor<"player_per_cube">;

    template<> bool visit_players::possible(const effect_context &ctx) {
        return defer<"player">().possible(ctx);
    }

    template<> player_list visit_players::random_target(const effect_context &ctx) {
        return get_all_player_targets(origin, origin_card, effect, ctx)
            | rv::sample(ctx.selected_cubes.count(origin_card) + 1)
            | rn::to_vector;
    }

    template<> game_string visit_players::get_error(const effect_context &ctx, const player_list &targets) {
        int num_cubes = ctx.selected_cubes.count(origin_card);
        if (targets.size() != num_cubes + 1) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player *target : targets) {
            MAYBE_RETURN(defer<"player">().get_error(ctx, target));
        }
        return {};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx, const player_list &targets) {
        game_string msg;
        for (player *target : targets) {
            msg = defer<"player">().prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_players::add_context(effect_context &ctx, const player_list &targets) {
        for (player *target : targets) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx, const player_list &targets) {
        for (player *target : targets) {
            defer<"player">().play(ctx, target);
        }
    }

}