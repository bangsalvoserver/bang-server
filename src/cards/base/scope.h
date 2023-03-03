#ifndef __BASE_SCOPE_H__
#define __BASE_SCOPE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_scope {
        int diff;
        equip_scope(int diff) : diff(diff) {}
        
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif