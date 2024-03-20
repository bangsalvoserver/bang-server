#include "game/play_verify.h"

#include "game/possible_to_play.h"

namespace banggame {

    using visit_player = play_visitor<target_type::conditional_player>;

    template<> game_string visit_player::get_error(const effect_context &ctx, player *target) {
        if (target) {
            return play_visitor<target_type::player>{origin, origin_card, effect}.get_error(ctx, target);
        } else if (contains_at_least(make_player_target_set(origin, origin_card, effect), 1)) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    template<> duplicate_set visit_player::duplicates(player *target) {
        if (target) {
            return {.players{target}};
        } else {
            return {};
        }
    }

    template<> game_string visit_player::prompt(const effect_context &ctx, player *target) {
        if (target) {
            return play_visitor<target_type::player>{origin, origin_card, effect}.prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void visit_player::add_context(effect_context &ctx, player *target) {
        if (target) {
            play_visitor<target_type::player>{origin, origin_card, effect}.add_context(ctx, target);
        }
    }

    template<> void visit_player::play(const effect_context &ctx, player *target) {
        if (target) {
            play_visitor<target_type::player>{origin, origin_card, effect}.play(ctx, target);
        }
    }

}