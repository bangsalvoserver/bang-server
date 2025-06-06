#ifndef __HIGHNOON_BLESSING_CURSE_H__
#define __HIGHNOON_BLESSING_CURSE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_blessing : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(blessing, equip_blessing)

    struct equip_curse : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(curse, equip_curse)
}

#endif