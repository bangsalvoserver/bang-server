// cyril.h
#ifndef __VLCATA_CYRIL_H__
#define __VLCATA_CYRIL_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_cyril : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(cyril, equip_cyril)
}
#endif