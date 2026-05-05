#ifndef __FRONTIER_COYOTES_H__
#define __FRONTIER_COYOTES_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_coyotes : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(coyotes, equip_coyotes)
}

#endif