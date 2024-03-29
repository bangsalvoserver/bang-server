#ifndef __ARMEDANDDANGEROUS_THUNDERER_H__
#define __ARMEDANDDANGEROUS_THUNDERER_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_thunderer {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(thunderer, effect_thunderer)
}

#endif