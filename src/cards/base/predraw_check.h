#ifndef __BASE_PREDRAW_CHECK_H__
#define __BASE_PREDRAW_CHECK_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_predraw_check {
        int priority;
        equip_predraw_check(int priority) : priority(priority) {}

        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);

        static void queue(player *target);
    };

}

#endif