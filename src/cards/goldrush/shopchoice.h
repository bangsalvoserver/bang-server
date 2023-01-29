#ifndef __GOLDRUSH_SHOPCHOICE_H__
#define __GOLDRUSH_SHOPCHOICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_shopchoice {
        game_string verify(card *origin_card, player *origin, card *playing_card, effect_context &ctx);
    };
}

#endif