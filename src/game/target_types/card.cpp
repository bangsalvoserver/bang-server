#include "game/play_verify.h"

#include "cards/filters.h"

namespace banggame {

    using visit_card = play_visitor<target_type::card>;

    template<> game_string visit_card::get_error(const effect_context &ctx, card *target) {
        if (target->owner) {
            MAYBE_RETURN(filters::check_player_filter(origin, effect.player_filter, target->owner, ctx));
        }
        MAYBE_RETURN(filters::check_card_filter(origin_card, origin, effect.card_filter, target, ctx));
        return effect.get_error(origin_card, origin, target, ctx);
    }

    template<> duplicate_set visit_card::duplicates(card *target) {
        if (bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            return {.cards{target}};
        }
    }

    template<> game_string visit_card::prompt(const effect_context &ctx, card *target) {
        return effect.on_prompt(origin_card, origin, target, ctx);
    }

    template<> void visit_card::add_context(effect_context &ctx, card *target) {
        effect.add_context(origin_card, origin, target, ctx);
    }

    template<> void visit_card::play(const effect_context &ctx, card *target) {
        auto flags = effect_flags::single_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target->owner != origin && target->pocket == pocket_type::player_hand) {
            effect.on_play(origin_card, origin, target->owner->random_hand_card(), flags, ctx);
        } else {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}