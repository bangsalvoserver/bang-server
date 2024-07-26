#ifndef __HIGHNOON_GHOSTTOWN_H__
#define __HIGHNOON_GHOSTTOWN_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_ghosttown : event_equip  {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(ghosttown, equip_ghosttown)
}

#endif