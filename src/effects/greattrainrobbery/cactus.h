#ifndef __GREATTRAINROBBERY_CACTUS_H__
#define __GREATTRAINROBBERY_CACTUS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_cactus {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(cactus, effect_cactus)
}

#endif