#ifndef __GOLDRUSH_LUCKYCHARM_H__
#define __GOLDRUSH_LUCKYCHARM_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_luckycharm : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif