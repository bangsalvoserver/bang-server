#ifndef __ARMEDANDDANGEROUS_RED_RINGO_H__
#define __ARMEDANDDANGEROUS_RED_RINGO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_red_ringo: event_equip {
        void on_enable(card *target_card, player *target);
    };

    struct effect_red_ringo {
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif