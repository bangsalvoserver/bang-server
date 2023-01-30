#include "leevankliff.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_leevankliff::verify(card *origin_card, player *origin, card *playing_card) {
        if (origin->m_played_cards.empty()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        auto modifiers = origin->m_played_cards.back().second
            | std::views::transform(&card_pocket_pair::origin_card);
        auto it = std::ranges::find(modifiers, card_modifier_type::leevankliff, &card::modifier_type);
        if (it != modifiers.end()) {
            return {"ERROR_CANNOT_REPEAT_CARD", *it};
        }

        return {};
    }

    void modifier_leevankliff::add_context(card *origin_card, player *origin, card *playing_card, effect_context &ctx) {
        ctx.disable_banglimit = true;
        ctx.repeating = true;
    }
}