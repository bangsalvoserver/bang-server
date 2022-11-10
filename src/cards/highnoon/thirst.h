#ifndef __HIGHNOON_THIRST_H__
#define __HIGHNOON_THIRST_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_thirst : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif