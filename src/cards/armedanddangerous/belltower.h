#ifndef __ARMEDANDDANGEROUS_BELLTOWER_H__
#define __ARMEDANDDANGEROUS_BELLTOWER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_belltower {
        bool valid_with_card(card *origin_card, player *origin, card *playing_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif