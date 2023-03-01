#include "moneybag.h"

#include "cards/effect_context.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_moneybag::get_error(card *origin_card, player *origin, card *playing_card) {
        if (playing_card->is_modifier()) {
            return "ERROR_NOT_ALLOWED_WITH_MODIFIER";
        }

        if (origin->m_game->m_discards.empty()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        card *target_card = origin->m_game->m_discards.back();

        if (target_card != playing_card || !playing_card->is_brown()) {
            return "INVALID_MODIFIER_CARD";
        }

        return {};
    }

    void modifier_moneybag::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
        ctx.repeating = true;
    }
}