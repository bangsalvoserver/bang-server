#include "leevankliff.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_leevankliff::verify(card *origin_card, player *origin, card *playing_card) {
        const auto &modifiers = origin->get_last_played_modifiers();
        auto it = std::ranges::find(modifiers, card_modifier_type::leevankliff, &card::modifier_type);
        if (it != modifiers.end()) {
            return {"ERROR_CANNOT_REPEAT_CARD", *it};
        }
        return {};
    }
}