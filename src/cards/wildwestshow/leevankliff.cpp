#include "leevankliff.h"

#include "cards/effect_context.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_leevankliff::get_error(card *origin_card, player *origin, card *playing_card) {
        if (playing_card->is_modifier()) {
            return "ERROR_NOT_ALLOWED_WITH_MODIFIER";
        }

        if (origin->m_played_cards.empty()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
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
        ctx.repeating = true;
    }
}