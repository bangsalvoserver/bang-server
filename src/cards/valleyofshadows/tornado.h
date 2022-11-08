#ifndef __VALLEYOFSHADOWS_TORNADO_H__
#define __VALLEYOFSHADOWS_TORNADO_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_tornado {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags);
    };
}

#endif