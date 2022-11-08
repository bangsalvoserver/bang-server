#ifndef __HIGHNOON_HIGHNOON_H__
#define __HIGHNOON_HIGHNOON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_highnoon : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif