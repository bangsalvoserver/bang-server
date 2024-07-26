#ifndef __WILDWESTSHOW_WILDWESTSHOW_H___
#define __WILDWESTSHOW_WILDWESTSHOW_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wildwestshow {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(wildwestshow, equip_wildwestshow)
}

#endif