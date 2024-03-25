#ifndef __ARMEDANDDANGEROUS_BIGFIFTY_H__
#define __ARMEDANDDANGEROUS_BIGFIFTY_H__

#include "cards/card_effect.h"
#include "cards/base/bang.h"

namespace banggame {

    struct effect_bigfifty {
        void on_play(card *origin_card, player *origin);
    };

    struct modifier_bigfifty : modifier_bangmod {
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(bigfifty, modifier_bigfifty)
}

#endif