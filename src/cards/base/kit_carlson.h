#ifndef __BASE_KIT_CARLSON_H__
#define __BASE_KIT_CARLSON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_kit_carlson : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif