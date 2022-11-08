#ifndef __GOLDRUSH_LUCKYCHARM_H__
#define __GOLDRUSH_LUCKYCHARM_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_luckycharm : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif