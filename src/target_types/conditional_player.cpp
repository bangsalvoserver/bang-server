#include "game/possible_to_play.h"

namespace banggame {

    using visit_player = play_visitor<"conditional_player">;

    template<> bool visit_player::possible(const effect_context &ctx) {
        return true;
    }

    template<> player *visit_player::random_target(const effect_context &ctx) {
        auto targets = get_all_player_targets(origin, origin_card, effect, ctx);
        if (targets.empty()) {
            return nullptr;
        } else {
            return random_element(targets, origin->m_game->bot_rng);
        }
    }

    template<> game_string visit_player::get_error(const effect_context &ctx, player *target) {
        if (target) {
            return defer<"player">().get_error(ctx, target);
        } else if (bool(get_all_player_targets(origin, origin_card, effect))) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    template<> game_string visit_player::prompt(const effect_context &ctx, player *target) {
        if (target) {
            return defer<"player">().prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void visit_player::add_context(effect_context &ctx, player *target) {
        if (target) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_player::play(const effect_context &ctx, player *target) {
        if (target) {
            defer<"player">().play(ctx, target);
        }
    }

}