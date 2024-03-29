#ifndef __CANYONDIABLO_DISARM_H__
#define __CANYONDIABLO_DISARM_H__

#include "cards/card_effect.h"
#include "effects/base/missed.h"

namespace banggame {

    struct effect_disarm : effect_missed {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(disarm, effect_disarm)
}

#endif