#ifndef __ARMEDANDDANGEROUS_PAY_CUBE_H__
#define __ARMEDANDDANGEROUS_PAY_CUBE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_pay_cube {
        void add_context(card *origin_card, player *origin, card *target, effect_context &ctx);
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(pay_cube, effect_pay_cube)
}

#endif