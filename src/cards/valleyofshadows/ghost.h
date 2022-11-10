#ifndef __VALLEYOFSHADOWS_GHOST_H__
#define __VALLEYOFSHADOWS_GHOST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ghost {
        void on_equip(card *target_card, player *target);
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };
}

#endif