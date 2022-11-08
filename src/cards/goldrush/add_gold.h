#ifndef __GOLDRUSH_ADD_GOLD_H__
#define __GOLDRUSH_ADD_GOLD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_add_gold {
        int amount;
        effect_add_gold(int value) : amount(std::max(1, value)) {}

        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif