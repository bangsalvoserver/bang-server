#ifndef __DODGECITY_TEQUILA_JOE_H__
#define __DODGECITY_TEQUILA_JOE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_tequila_joe : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(tequila_joe, equip_tequila_joe)
}

#endif