#ifndef __CANYONDIABLO_WYATT_EARL_H__
#define __CANYONDIABLO_WYATT_EARL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wyatt_earl : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(wyatt_earl, equip_wyatt_earl)
}

#endif