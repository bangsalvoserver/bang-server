#ifndef __ARMEDANDDANGEROUS_BIGFIFTY_H__
#define __ARMEDANDDANGEROUS_BIGFIFTY_H__

#include "cards/card_effect.h"
#include "effects/base/bang.h"

namespace banggame {

    struct effect_bigfifty {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(bigfifty, effect_bigfifty)

    struct modifier_bigfifty : modifier_bangmod {
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(bigfifty, modifier_bigfifty)
}

#endif