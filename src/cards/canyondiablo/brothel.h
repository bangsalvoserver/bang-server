#ifndef __CANYONDIABLO_BROTHEL_H__
#define __CANYONDIABLO_BROTHEL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_brothel : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif