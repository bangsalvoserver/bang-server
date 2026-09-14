#ifndef __VLCATA_KOTEL_H__
#define __VLCATA_KOTEL_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_kotel : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(kotel, equip_kotel)
}

#endif