#ifndef __GOLDRUSH_DUTCH_WILL_H__
#define __GOLDRUSH_DUTCH_WILL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_dutch_will : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif