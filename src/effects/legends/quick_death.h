#ifndef __LEGENDS_QUICK_DEATH_H__
#define __LEGENDS_QUICK_DEATH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_quick_death : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(quick_death, equip_quick_death)
}

#endif