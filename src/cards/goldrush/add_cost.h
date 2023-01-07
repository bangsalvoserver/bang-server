#ifndef __GOLDRUSH_ADD_COST_H__
#define __GOLDRUSH_ADD_COST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_add_cost {
        int value;
        effect_add_cost(int value) : value(value) {}
        
        verify_result verify(card *origin_card, player *origin);
    };
}

#endif