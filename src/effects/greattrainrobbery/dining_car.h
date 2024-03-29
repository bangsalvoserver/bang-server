#ifndef __GREATTRAINROBBERY_DINING_CAR_H__
#define __GREATTRAINROBBERY_DINING_CAR_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_dining_car : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    DEFINE_EQUIP(dining_car, equip_dining_car)
}

#endif