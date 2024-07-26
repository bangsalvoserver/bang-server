#ifndef __FISTFULOFCARDS_FISTFULOFCARDS_H__
#define __FISTFULOFCARDS_FISTFULOFCARDS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_fistfulofcards : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(fistfulofcards, equip_fistfulofcards)
}

#endif