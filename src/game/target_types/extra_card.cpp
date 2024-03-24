#include "game/play_verify.h"

#include "cards/filter_enums.h"

namespace banggame {
    
    using visit_card = play_visitor<target_type::extra_card>;

    template<> game_string visit_card::get_error(const effect_context &ctx, card *target) {
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
                return play_visitor<target_type::card>{origin, origin_card, effect}.get_error(ctx, target);
            }
        }
    }

    template<> game_string visit_card::prompt(const effect_context &ctx, card *target) {
        if (target) {
            return play_visitor<target_type::card>{origin, origin_card, effect}.prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void visit_card::add_context(effect_context &ctx, card *target) {
        if (target) {
            play_visitor<target_type::card>{origin, origin_card, effect}.add_context(ctx, target);
        }
    }

    template<> void visit_card::play(const effect_context &ctx, card *target) {
        if (target) {
            return play_visitor<target_type::card>{origin, origin_card, effect}.play(ctx, target);
        }
    }

}