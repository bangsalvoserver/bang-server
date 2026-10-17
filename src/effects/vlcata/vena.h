#ifndef __VLCATA_VENA_H__
#define __VLCATA_VENA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_vena : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(vena, equip_vena)
}

#endif