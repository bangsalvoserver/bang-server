#ifndef __VALLEYOFSHADOWS_AIM_H__
#define __VALLEYOFSHADOWS_AIM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_aim {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(aim, effect_aim)
}

#endif