#ifndef __GOLDRUSH_GUNBELT_H__
#define __GOLDRUSH_GUNBELT_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_gunbelt : event_based_effect {
        int ncards;
        effect_gunbelt(int value) : ncards(value) {}
        
        void on_enable(card *target_card, player *target);
    };
}

#endif