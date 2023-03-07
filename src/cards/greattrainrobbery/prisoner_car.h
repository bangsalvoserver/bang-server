#ifndef __GREATTRAINROBBERY_PRISONER_CAR_H__
#define __GREATTRAINROBBERY_PRISONER_CAR_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_prisoner_car : event_equip {
        void on_enable(card *origin_card, player *origin);
    };
}

#endif