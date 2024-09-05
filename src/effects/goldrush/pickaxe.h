#ifndef __GOLDRUSH_PICKAXE_H__
#define __GOLDRUSH_PICKAXE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_pickaxe : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(pickaxe, equip_pickaxe)
}

#endif