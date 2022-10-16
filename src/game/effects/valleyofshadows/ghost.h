#ifndef __VALLEYOFSHADOWS_GHOST_H__
#define __VALLEYOFSHADOWS_GHOST_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_ghost {
        void on_equip(card *target_card, player *target);
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };
}

#endif