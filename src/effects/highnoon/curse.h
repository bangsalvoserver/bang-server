#ifndef __HIGHNOON_CURSE_H__
#define __HIGHNOON_CURSE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_curse : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(curse, equip_curse)
}

#endif