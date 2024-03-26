#ifndef __BASE_VOLCANIC_H__
#define __BASE_VOLCANIC_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_volcanic : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(volcanic, equip_volcanic)
}

#endif