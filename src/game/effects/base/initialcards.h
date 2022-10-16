#ifndef __BASE_INITIALCARDS_H__
#define __BASE_INITIALCARDS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_initialcards : event_based_effect {
        int value;
        effect_initialcards(int value) : value(value) {}
        
        void on_enable(card *target_card, player *target);
    };
}

#endif