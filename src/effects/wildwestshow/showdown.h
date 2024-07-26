#ifndef __WILDWESTSHOW_SHOWDOWN_H___
#define __WILDWESTSHOW_SHOWDOWN_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_showdown {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(showdown, equip_showdown)
}

#endif