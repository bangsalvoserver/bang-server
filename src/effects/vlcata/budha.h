#ifndef __VLCATA_BUDHA_H__
#define __VLCATA_BUDHA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_budha : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(budha, equip_budha)
}

#endif