#ifndef __HIGHNOON_HIGHNOON_H__
#define __HIGHNOON_HIGHNOON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_highnoon : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(highnoon, equip_highnoon)
}

#endif