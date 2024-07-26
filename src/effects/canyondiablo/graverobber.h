#ifndef __CANYONDIABLO_GRAVEROBBER_H__
#define __CANYONDIABLO_GRAVEROBBER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_graverobber {
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(graverobber, effect_graverobber)
}

#endif