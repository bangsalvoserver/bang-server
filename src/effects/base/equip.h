#ifndef __BASE_EQUIP_H__
#define __BASE_EQUIP_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_equip {
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx, card *target_card, player *target);
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *target_card, player *target);
        void on_play(card *origin_card, player *origin, const effect_context &ctx, card *target_card, player *target);
    };

    DEFINE_MTH(equip, handler_equip)
}

#endif