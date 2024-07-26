#ifndef __CANYONDIABLO_MIRAGE_H__
#define __CANYONDIABLO_MIRAGE_H__

#include "cards/card_effect.h"
#include "effects/base/missed.h"

namespace banggame {

    struct effect_mirage : effect_missed {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(mirage, effect_mirage)
}

#endif