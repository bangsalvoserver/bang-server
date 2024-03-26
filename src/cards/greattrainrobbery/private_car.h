#ifndef __GREATTRAINROBBERY_PRIVATE_CAR_H__
#define __GREATTRAINROBBERY_PRIVATE_CAR_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_private_car : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    DEFINE_EQUIP(private_car, equip_private_car)
}

#endif