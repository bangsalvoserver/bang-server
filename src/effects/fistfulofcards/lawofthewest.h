#ifndef __FISTFULOFCARDS_LAWOFTHEWEST_H__
#define __FISTFULOFCARDS_LAWOFTHEWEST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lawofthewest : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(lawofthewest, equip_lawofthewest)
}

#endif