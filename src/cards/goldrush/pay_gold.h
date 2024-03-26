#ifndef __GOLDRUSH_PAY_GOLD_H__
#define __GOLDRUSH_PAY_GOLD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_pay_gold {
        int amount;
        effect_pay_gold(int value) : amount(value) {}

        game_string get_error(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(pay_gold, effect_pay_gold)
}

#endif