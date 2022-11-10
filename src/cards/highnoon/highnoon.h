#ifndef __HIGHNOON_HIGHNOON_H__
#define __HIGHNOON_HIGHNOON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_highnoon : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif