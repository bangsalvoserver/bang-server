#ifndef __HIGHNOON_TRAINARRIVAL_H__
#define __HIGHNOON_TRAINARRIVAL_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_trainarrival : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif