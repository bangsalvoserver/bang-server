#ifndef __BASE_VOLCANIC_H__
#define __BASE_VOLCANIC_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_volcanic {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(volcanic, equip_volcanic)
}

#endif