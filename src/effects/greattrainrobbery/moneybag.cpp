#include "moneybag.h"

#include "cards/filter_enums.h"

#include "effects/base/bang.h"
#include "effects/wildwestshow/leevankliff.h"

#include "game/game_table.h"

namespace banggame {

    game_string modifier_moneybag::get_error(card_ptr origin_card, player_ptr origin, card_ptr playing_card, const effect_context &ctx) {
        card_ptr repeat_card = ctx.get<contexts::repeat_card>();
        if (!repeat_card) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        playing_card = get_repeat_playing_card(playing_card, ctx);
        
        if (repeat_card != playing_card) {
            return "INVALID_MODIFIER_CARD";
        }

        if (!playing_card->is_brown()) {
            return "ERROR_CARD_IS_NOT_BROWN";
        }

        return {};
    }

    void modifier_moneybag::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.get<contexts::disable_banglimit>() = true;
        if (!origin->m_game->m_discards.empty()) {
            ctx.get<contexts::repeat_card>() = origin->m_game->m_discards.back();
        }
    }
}