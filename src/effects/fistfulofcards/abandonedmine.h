#ifndef __FISTFULOFCARDS_ABANDONEDMINE_H__
#define __FISTFULOFCARDS_ABANDONEDMINE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_abandonedmine {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(abandonedmine, equip_abandonedmine)
}

#endif