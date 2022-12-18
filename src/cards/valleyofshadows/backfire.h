#ifndef __VALLEYOFSHADOWS_BACKFIRE_H__
#define __VALLEYOFSHADOWS_BACKFIRE_H__

#include "cards/card_effect.h"
#include "cards/base/missed.h"

namespace banggame {
    
    struct effect_backfire : effect_missed {
        void on_play(card *origin_card, player *origin);
    };
}

#endif