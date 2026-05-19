#ifndef __FRONTIER_COFFIN_H__
#define __FRONTIER_COFFIN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_coffin {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(coffin, equip_coffin)

    struct equip_coffin_nodisable {
        struct nodisable{};
        
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(coffin_nodisable, equip_coffin_nodisable)
}

#endif