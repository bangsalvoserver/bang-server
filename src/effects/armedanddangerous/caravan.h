#ifndef __ARMEDANDDANGEROUS_CARAVAN_H__
#define __ARMEDANDDANGEROUS_CARAVAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_caravan {
        int num_cubes;
        effect_caravan(int num_cubes) : num_cubes{num_cubes} {}
        
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(caravan, effect_caravan)
}

#endif