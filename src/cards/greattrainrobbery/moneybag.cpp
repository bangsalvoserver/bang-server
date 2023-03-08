#include "moneybag.h"

#include "cards/effect_context.h"

#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_moneybag::get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) {
        if (!ctx.repeat_card) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        if (ctx.card_choice) {
            playing_card = ctx.card_choice;
        } else if (ctx.traincost) {
            playing_card = ctx.traincost;
        }
        
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