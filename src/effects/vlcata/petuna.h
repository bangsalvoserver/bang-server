#ifndef __VLCATA_PETUNA_H__
#define __VLCATA_PETUNA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_petuna : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(petuna, equip_petuna)
}

#endif