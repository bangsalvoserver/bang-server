#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<"select_cubes_optional">;

    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return true;
    }

    template<> card_list visit_cubes::random_target(const effect_context &ctx) {
        auto deferred = defer<"select_cubes">();
        if (deferred.possible(ctx)) {
            return deferred.random_target(ctx);
        }
        return {};
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (!target_cards.empty() && target_cards.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : target_cards) {
            if (c->owner != origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx, const card_list &target_cards) {
        return defer<"select_cubes">().prompt(ctx, target_cards);
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const card_list &target_cards) {
        defer<"select_cubes">().add_context(ctx, target_cards);
    }

    template<> void visit_cubes::play(const effect_context &ctx, const card_list &target_cards) {
        defer<"select_cubes">().play(ctx, target_cards);
    }

}