#ifndef __GOLDRUSH_DUTCH_WILL_H__
#define __GOLDRUSH_DUTCH_WILL_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_dutch_will : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif