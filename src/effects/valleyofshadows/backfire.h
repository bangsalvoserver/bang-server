#ifndef __VALLEYOFSHADOWS_BACKFIRE_H__
#define __VALLEYOFSHADOWS_BACKFIRE_H__

#include "cards/card_effect.h"
#include "effects/base/missed.h"

namespace banggame {
    
    struct effect_backfire : effect_missed {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(backfire, effect_backfire)
}

#endif