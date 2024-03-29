#include "moneybag.h"

#include "cards/filter_enums.h"

#include "effects/wildwestshow/leevankliff.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_moneybag::get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) {
        if (!ctx.repeat_card) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        playing_card = get_repeat_playing_card(playing_card, ctx);
        
        if (ctx.repeat_card != playing_card) {
            return "INVALID_MODIFIER_CARD";
        }

        if (!playing_card->is_brown()) {
            return "ERROR_CARD_IS_NOT_BROWN";
        }

        return {};
    }

    void modifier_moneybag::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
        if (!origin->m_game->m_discards.empty()) {
            ctx.repeat_card = origin->m_game->m_discards.back();
        }
    }
}