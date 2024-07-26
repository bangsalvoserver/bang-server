#ifndef __THEBULLET_NEWIDENTITY_H__
#define __THEBULLET_NEWIDENTITY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_newidentity : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(newidentity, equip_newidentity)

}

#endif