#ifndef __HIGHNOON_BLESSING_H__
#define __HIGHNOON_BLESSING_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_blessing : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif