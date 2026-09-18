#ifndef __VLCATA_VERCA_H__
#define __VLCATA_VERCA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_verca_instant_green : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(verca_instant_green, equip_verca_instant_green)
}

#endif