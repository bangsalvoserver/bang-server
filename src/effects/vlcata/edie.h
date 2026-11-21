#ifndef __VLCATA_EDIE_H__
#define __VLCATA_EDIE_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_edie : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(edie, equip_edie)
}

#endif