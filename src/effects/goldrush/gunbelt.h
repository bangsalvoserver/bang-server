#ifndef __GOLDRUSH_GUNBELT_H__
#define __GOLDRUSH_GUNBELT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_gunbelt : event_equip {
        int ncards;
        equip_gunbelt(int value) : ncards(value) {}
        
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(gunbelt, equip_gunbelt)
}

#endif