#include "game/play_verify.h"

namespace banggame {

    using visit_player = play_visitor<target_type::conditional_player>;

    template<> bool visit_player::possible(const effect_context &ctx) {
        return true;
    }

    template<> player *visit_player::random_target(const effect_context &ctx) {
        auto targets = make_player_target_set(origin, origin_card, effect, ctx) | rn::to_vector;
        if (targets.empty()) {
            return nullptr;
        } else {
            return random_element(targets, origin->m_game->rng);
        }
    }

    template<> game_string visit_player::get_error(const effect_context &ctx, player *target) {
        if (target) {
            return defer<target_type::player>().get_error(ctx, target);
        } else if (contains_at_least(make_player_target_set(origin, origin_card, effect), 1)) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    template<> game_string visit_player::prompt(const effect_context &ctx, player *target) {
        if (target) {
            return defer<target_type::player>().prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void visit_player::add_context(effect_context &ctx, player *target) {
        if (target) {
            defer<target_type::player>().add_context(ctx, target);
        }
    }

    template<> void visit_player::play(const effect_context &ctx, player *target) {
        if (target) {
            defer<target_type::player>().play(ctx, target);
        }
    }

}