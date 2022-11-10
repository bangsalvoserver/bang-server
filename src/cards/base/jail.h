#ifndef __BASE_JAIL_H__
#define __BASE_JAIL_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_jail : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif