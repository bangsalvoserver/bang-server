#ifndef __BASE_PREDRAW_CHECK_H__
#define __BASE_PREDRAW_CHECK_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_predraw_check {
        int priority;
        effect_predraw_check(int priority) : priority(priority) {}

        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);

        static void queue(player *target);
    };

}

#endif