#ifndef __HIGHNOON_THIRST_H__
#define __HIGHNOON_THIRST_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_thirst : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(thirst, equip_thirst)
}

#endif