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

    template<> duplicate_set visit_card::duplicates(card *target) {
        if (!target || bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            return {.cards{target}};
        }
    }

    template<> game_string visit_card::prompt(const effect_context &ctx, card *target) {
        if (target) {
            return effect.on_prompt(origin_card, origin, target, ctx);
        } else {
            return {};
        }
    }

    template<> void visit_card::add_context(effect_context &ctx, card *target) {
        if (target) {
            effect.add_context(origin_card, origin, target, ctx);
        }
    }

    template<> void visit_card::play(const effect_context &ctx, card *target) {
        if (target) {
            effect.on_play(origin_card, origin, target, {}, ctx);
        }
    }

}