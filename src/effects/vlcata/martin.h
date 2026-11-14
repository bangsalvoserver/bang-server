// martin.h
#ifndef __VLCATA_MARTIN_H__
#define __VLCATA_MARTIN_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_martin : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(martin, equip_martin)
}
#endif