#ifndef __ARMEDANDDANGEROUS_FLINTLOCK_H__
#define __ARMEDANDDANGEROUS_FLINTLOCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_flintlock {
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(flintlock, effect_flintlock)
}

#endif