#ifndef __BASE_KIT_CARLSON_H__
#define __BASE_KIT_CARLSON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_kit_carlson : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif