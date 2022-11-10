#ifndef __CANYONDIABLO_BRONCO_H__
#define __CANYONDIABLO_BRONCO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bronco {
        void on_equip(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };
}

#endif