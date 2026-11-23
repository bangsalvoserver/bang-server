#ifndef __ARMEDANDDANGEROUS_FLINTLOCK_H__
#define __ARMEDANDDANGEROUS_FLINTLOCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_flintlock {
        int evaluate(card_ptr origin_card, player_ptr origin, int ncubes) const;
        void on_play(card_ptr origin_card, player_ptr origin, int ncubes) const;
    };

    DEFINE_EFFECT(flintlock, effect_flintlock)
}

#endif