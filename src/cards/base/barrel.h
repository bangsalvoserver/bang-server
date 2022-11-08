#ifndef __BASE_BARREL_H__
#define __BASE_BARREL_H__

#include "missed.h"

namespace banggame {
    
    struct effect_barrel : effect_missed {
        void on_play(card *origin_card, player *target);
    };
}

#endif