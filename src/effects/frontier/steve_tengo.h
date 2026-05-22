#ifndef __FRONTIER_STEVE_TENGO_H__
#define __FRONTIER_STEVE_TENGO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_steve_tengo {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(steve_tengo, equip_steve_tengo)

    struct equip_steve_tengo_nodisable {
        struct nodisable{};
        
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(steve_tengo_nodisable, equip_steve_tengo_nodisable)
}

#endif