#ifndef __VLCATA_PATKA_H__
#define __VLCATA_PATKA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_patka : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(patka, equip_patka)
}

#endif