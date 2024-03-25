#include "game/play_verify.h"

#include "game/possible_to_play.h"
#include "cards/filters.h"

namespace banggame {

    using visit_players = play_visitor<target_type::player_per_cube>;

    template<> bool visit_players::possible(const effect_context &ctx) {
        return defer<target_type::player>().possible(ctx);
    }

    template<> serial::player_list visit_players::random_target(const effect_context &ctx) {
        size_t num_cubes = filters::get_selected_cubes(origin_card, ctx).size();
        auto targets = make_player_target_set(origin, origin_card, effect, ctx) | rn::to_vector;
        return targets
            | rv::sample(num_cubes + 1)
            | rn::to<serial::player_list>;
    }

    template<> game_string visit_players::get_error(const effect_context &ctx, const serial::player_list &targets) {
        int num_cubes = filters::get_selected_cubes(origin_card, ctx).size();
        if (targets.size() != num_cubes + 1) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player *target : targets) {
            MAYBE_RETURN(defer<target_type::player>().get_error(ctx, target));
        }
        return {};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx, const serial::player_list &targets) {
        game_string msg;
        for (player *target : targets) {
            msg = defer<target_type::player>().prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_players::add_context(effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            defer<target_type::player>().add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            defer<target_type::player>().play(ctx, target);
        }
    }

}