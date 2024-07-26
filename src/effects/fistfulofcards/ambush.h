#ifndef __FISTFULOFCARDS_AMBUSH_H__
#define __FISTFULOFCARDS_AMBUSH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ambush {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(ambush, equip_ambush)
}

#endif