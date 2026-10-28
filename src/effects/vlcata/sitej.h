#ifndef __VLCATA_SITEJ_H__
#define __VLCATA_SITEJ_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_sitej : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(sitej, equip_sitej)
}

#endif