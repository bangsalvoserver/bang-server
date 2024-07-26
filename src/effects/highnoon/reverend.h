#ifndef __HIGHNOON_REVEREND_H__
#define __HIGHNOON_REVEREND_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_reverend {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(reverend, equip_reverend)
}

#endif