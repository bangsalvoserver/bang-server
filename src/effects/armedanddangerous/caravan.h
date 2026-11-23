#ifndef __ARMEDANDDANGEROUS_CARAVAN_H__
#define __ARMEDANDDANGEROUS_CARAVAN_H__

#include "cards/card_effect.h"

#include "effects/base/draw.h"

namespace banggame {

    struct effect_caravan {
        int ncards;
        effect_caravan(int ncards): ncards{ncards} {}

        int evaluate(card_ptr origin_card, player_ptr origin, int ncubes) const {
            return effect_draw{ncards + ncubes}.evaluate(origin_card, origin);
        }

        void on_play(card_ptr origin_card, player_ptr origin, int ncubes) const {
            return effect_draw{ncards + ncubes}.on_play(origin_card, origin);
        }
    };

    DEFINE_EFFECT(caravan, effect_caravan)
}

#endif