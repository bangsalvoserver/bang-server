#ifndef __VLCATA_HOROUS_H__
#define __VLCATA_HOROUS_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_horous : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(horous, equip_horous)
}

#endif