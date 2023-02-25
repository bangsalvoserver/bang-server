#include "leevankliff.h"

#include "game/game.h"

namespace banggame {

    game_string effect_leevankliff::get_error(card *origin_card, player *origin) {
        if (origin->m_played_cards.empty()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        if (ranges::contains(origin->m_played_cards.back().second, origin_card, &card_pocket_pair::origin_card)) {
            return {"ERROR_CANNOT_REPEAT_CARD", origin_card};
        }

        return {};
    }

    bool modifier_leevankliff::valid_with_card(card *origin_card, player *origin, card *target_card) {
        return target_card->is_brown() && target_card == origin->get_last_played_card();
    }

    void modifier_leevankliff::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
        ctx.repeating = true;
    }
}