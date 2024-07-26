#ifndef __CANYONDIABLO_INDIANGUIDE_H__
#define __CANYONDIABLO_INDIANGUIDE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_indianguide : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(indianguide, equip_indianguide)
}

#endif