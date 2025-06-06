#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {
    
    using visit_card = play_visitor<target_types::extra_card>;

    template<> std::generator<nullable_card> visit_card::possible_targets(const effect_context &ctx) {
        if (ctx.repeat_card) {
            co_yield nullptr;
        } else {
            for (card_ptr target : get_all_card_targets(origin, origin_card, effect, ctx)) {
                co_yield target;
            }
        }
    }

    template<> nullable_card visit_card::random_target(const effect_context &ctx) {
        if (ctx.repeat_card) {
            return nullptr;
        } else {
            return defer<target_types::card>().random_target(ctx);
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
                return defer<target_types::card>().get_error(ctx, target);
            }
        }
    }

    template<> prompt_string visit_card::prompt(const effect_context &ctx, nullable_card target) {
        if (target) {
            return defer<target_types::card>().prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void visit_card::add_context(effect_context &ctx, nullable_card target) {
        if (target) {
            defer<target_types::card>().add_context(ctx, target);
        }
    }

    template<> void visit_card::play(const effect_context &ctx, nullable_card target) {
        if (target) {
            return defer<target_types::card>().play(ctx, target);
        }
    }

}