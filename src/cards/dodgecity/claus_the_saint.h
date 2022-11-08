#ifndef __DODGECITY_CLAUS_THE_SAINT_H__
#define __DODGECITY_CLAUS_THE_SAINT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_claus_the_saint : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif