#ifndef __FISTFULOFCARDS_DEADMAN_H__
#define __FISTFULOFCARDS_DEADMAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_deadman : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(deadman, equip_deadman)
}

#endif