#ifndef __GOLDRUSH_GOLDPAN_H__
#define __GOLDRUSH_GOLDPAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_pay_gold {
        int amount;
        effect_pay_gold(int value) : amount(value) {}

        game_string get_error(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(pay_gold, effect_pay_gold)

    struct equip_goldpan {
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(goldpan, equip_goldpan)

    struct effect_goldpan {
        int max_usages;

        effect_goldpan(int max_usages): max_usages{max_usages} {}

        game_string get_error(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(goldpan, effect_goldpan)
}

#endif