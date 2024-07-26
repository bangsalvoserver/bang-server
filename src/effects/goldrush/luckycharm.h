#ifndef __GOLDRUSH_LUCKYCHARM_H__
#define __GOLDRUSH_LUCKYCHARM_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_luckycharm : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(luckycharm, equip_luckycharm)
}

#endif