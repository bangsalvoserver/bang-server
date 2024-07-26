#ifndef __WILDWESTSHOW_SACAGAWAY_H___
#define __WILDWESTSHOW_SACAGAWAY_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_sacagaway {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(sacagaway, equip_sacagaway)
}

#endif