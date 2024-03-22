#include "game/play_verify.h"

#include "game/possible_to_play.h"
#include "cards/filters.h"

namespace banggame {

    using visit_players = play_visitor<target_type::player_per_cube>;

    template<> game_string visit_players::get_error(const effect_context &ctx, const serial::player_list &targets) {
        int num_cubes = filters::get_selected_cubes(origin_card, ctx).size();
        if (targets.size() != num_cubes + 1) {
            return "ERROR_INVALID_TARGETS";
        }
        for (player *target : targets) {
            MAYBE_RETURN(play_visitor<target_type::player>{origin, origin_card, effect}.get_error(ctx, target));
        }
        return {};
    }

    template<> duplicate_set visit_players::duplicates(const serial::player_list &targets) {
        return {.players{ targets | rn::to<std::vector<player *>> }};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx, const serial::player_list &targets) {
        game_string msg;
        for (player *target : targets) {
            msg = play_visitor<target_type::player>{origin, origin_card, effect}.prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_players::add_context(effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            play_visitor<target_type::player>{origin, origin_card, effect}.add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            play_visitor<target_type::player>{origin, origin_card, effect}.play(ctx, target);
        }
    }

}