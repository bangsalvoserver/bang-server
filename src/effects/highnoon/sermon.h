#ifndef __HIGHNOON_SERMON_H__
#define __HIGHNOON_SERMON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_sermon {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(sermon, equip_sermon)
}

#endif