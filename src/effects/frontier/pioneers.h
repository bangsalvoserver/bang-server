#ifndef __FRONTIER_PIONEERS_H__
#define __FRONTIER_PIONEERS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_pioneers {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(pioneers, equip_pioneers)
}

#endif