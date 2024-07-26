#ifndef __BASE_BARREL_H__
#define __BASE_BARREL_H__

#include "missed.h"

namespace banggame {
    
    struct effect_barrel : effect_missed {
        void on_play(card_ptr origin_card, player_ptr target);
    };

    DEFINE_EFFECT(barrel, effect_barrel)
}

#endif