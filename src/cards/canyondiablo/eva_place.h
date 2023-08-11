#ifndef __CANYONDIABLO_EVA_PLACE_H__
#define __CANYONDIABLO_EVA_PLACE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_eva_place {
        void on_play(card *origin_card, player *origin);
    };
}

#endif