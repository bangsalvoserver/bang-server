#ifndef __DODGECITY_GREG_DIGGER_H__
#define __DODGECITY_GREG_DIGGER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_greg_digger : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(greg_digger, equip_greg_digger)
}

#endif