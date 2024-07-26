#ifndef __VALLEYOFSHADOWS_TORNADO_H__
#define __VALLEYOFSHADOWS_TORNADO_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_tornado {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(tornado, effect_tornado)
}

#endif