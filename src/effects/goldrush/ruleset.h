#ifndef __GOLDRUSH_RULESET_H__
#define __GOLDRUSH_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_goldrush {
        void on_apply(game *game);
    };

    DEFINE_RULESET(goldrush, ruleset_goldrush)

    struct effect_add_gold {
        int amount;
        effect_add_gold(int value) : amount(std::max(1, value)) {}

        void on_play(card_ptr origin_card, player_ptr origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(add_gold, effect_add_gold)

}

#endif