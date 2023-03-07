#include "leevankliff.h"

#include "cards/effect_context.h"
#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    bool modifier_leevankliff::valid_with_modifier(card *origin_card, player *origin, card *playing_card) {
        return playing_card->has_tag(tag_type::card_choice);
    }

    game_string modifier_leevankliff::get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) {
        if (origin->m_played_cards.empty()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        if (ctx.card_choice && playing_card->get_tag_value(tag_type::card_choice) == ctx.card_choice->get_tag_value(tag_type::card_choice)) {
            return {};
        }

        const auto &[target_card, modifiers] = origin->m_played_cards.back();

        if (target_card.origin_card != playing_card || !playing_card->is_brown()) {
            return "INVALID_MODIFIER_CARD";
        }

        if (ranges::contains(modifiers, origin_card, &card_pocket_pair::origin_card)) {
            return {"ERROR_CANNOT_REPEAT_CARD", origin_card};
        }

        return {};
    }

    void modifier_leevankliff::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
        ctx.repeat_card = origin->get_last_played_card();
    }
}