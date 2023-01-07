#include "leevankliff.h"

#include "game/game.h"

namespace banggame {

    game_string effect_leevankliff::verify(card *origin_card, player *origin) {
        const auto &modifiers = origin->get_last_played_card().second;
        auto it = std::ranges::find(modifiers, card_modifier_type::leevankliff, &card::modifier);
        if (it != modifiers.end()) {
            return {"ERROR_CANNOT_REPEAT_CARD", *it};
        }
        return {};
    }
}