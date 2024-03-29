#ifndef __CANYONDIABLO_UPDATE_MAX_HP_H__
#define __CANYONDIABLO_UPDATE_MAX_HP_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_update_max_hp {
        int value;
        equip_update_max_hp(int value): value{value} {}
        
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(update_max_hp, equip_update_max_hp)
}

#endif