#ifndef __BASE_VOLCANIC_H__
#define __BASE_VOLCANIC_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_volcanic : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif