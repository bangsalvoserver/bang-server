#ifndef __VALLEYOFSHADOWS_SHOTGUN_H__
#define __VALLEYOFSHADOWS_SHOTGUN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_shotgun : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(shotgun, equip_shotgun)
}

#endif