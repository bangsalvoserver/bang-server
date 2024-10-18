#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {
    
    using visit_card = play_visitor<"extra_card">;

    template<> bool visit_card::possible(const effect_context &ctx) {
        return ctx.repeat_card || bool(get_all_card_targets(origin, origin_card, effect, ctx));
    }

    template<> nullable_card visit_card::random_target(const effect_context &ctx) {
        if (ctx.repeat_card) {
            return nullptr;
        } else {
            return defer<"card">().random_target(ctx);
        }
    }

    template<> game_string visit_card::get_error(const effect_context &ctx, nullable_card target) {
        if (!target) {
            if (ctx.repeat_card) {
                return {};
            } else {
                return "ERROR_TARGET_SET_NOT_EMPTY";
            }
        } else {
            if (ctx.repeat_card) {
                return "ERROR_TARGET_SET_EMPTY";
            } else {
                return defer<"card">().get_error(ctx, target);
            }
        }
    }

    template<> prompt_string visit_card::prompt(const effect_context &ctx, nullable_card target) {
        if (target) {
            return defer<"card">().prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void visit_card::add_context(effect_context &ctx, nullable_card target) {
        if (target) {
            defer<"card">().add_context(ctx, target);
        }
    }

    template<> void visit_card::play(const effect_context &ctx, nullable_card target) {
        if (target) {
            return defer<"card">().play(ctx, target);
        }
    }

}