#include "game/play_verify.h"

#include "game/possible_to_play.h"

namespace banggame {

    using visit_players = play_visitor<"player_per_cube">;

    template<> bool visit_players::possible(const effect_context &ctx) {
        return defer<"player">().possible(ctx);
    }

    template<> serial::player_list visit_players::random_target(const effect_context &ctx) {
        int num_cubes = ctx.selected_cubes.count(origin_card);
        auto targets = make_player_target_set(origin, origin_card, effect, ctx) | rn::to_vector;
        return targets
            | rv::sample(num_cubes + 1)
            | rn::to<serial::player_list>;
    }

    template<> game_string visit_players::get_error(const effect_context &ctx, const serial::player_list &targets) {
        int num_cubes = ctx.selected_cubes.count(origin_card);
        if (targets.size() != num_cubes + 1) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player *target : targets) {
            MAYBE_RETURN(defer<"player">().get_error(ctx, target));
        }
        return {};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx, const serial::player_list &targets) {
        game_string msg;
        for (player *target : targets) {
            msg = defer<"player">().prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_players::add_context(effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            defer<"player">().play(ctx, target);
        }
    }

}