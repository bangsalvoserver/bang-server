#ifndef __BASE_JAIL_H__
#define __BASE_JAIL_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_jail : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif