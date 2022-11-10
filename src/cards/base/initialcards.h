#ifndef __BASE_INITIALCARDS_H__
#define __BASE_INITIALCARDS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_initialcards : event_equip {
        int value;
        equip_initialcards(int value) : value(value) {}
        
        void on_enable(card *target_card, player *target);
    };
}

#endif