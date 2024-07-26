#ifndef __VALLEYOFSHADOWS_BACKFIRE_H__
#define __VALLEYOFSHADOWS_BACKFIRE_H__

#include "cards/card_effect.h"
#include "effects/base/missed.h"

namespace banggame {
    
    struct effect_backfire : effect_missed {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(backfire, effect_backfire)
}

#endif