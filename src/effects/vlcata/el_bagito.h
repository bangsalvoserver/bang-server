#ifndef __VLCATA_EL_BAGITO_H__
#define __VLCATA_EL_BAGITO_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_el_bagito : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(el_bagito, equip_el_bagito)
}

#endif