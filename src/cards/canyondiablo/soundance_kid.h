#ifndef __CANYONDIABLO_SOUNDANCE_KID_H__
#define __CANYONDIABLO_SOUNDANCE_KID_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_soundance_kid : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(soundance_kid, equip_soundance_kid)
}

#endif