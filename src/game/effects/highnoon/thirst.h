#ifndef __HIGHNOON_THIRST_H__
#define __HIGHNOON_THIRST_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_thirst : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif