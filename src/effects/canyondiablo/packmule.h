#ifndef __CANYONDIABLO_PACKMULE_H__
#define __CANYONDIABLO_PACKMULE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_packmule : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(packmule, equip_packmule)
}

#endif