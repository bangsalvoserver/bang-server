#ifndef __GOLDRUSH_RULESET_H__
#define __GOLDRUSH_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_goldrush {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(goldrush, ruleset_goldrush)

    struct effect_add_gold {
        int amount;
        effect_add_gold(int amount): amount{amount} {}

        void on_play(card_ptr origin_card, player_ptr origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(add_gold, effect_add_gold)

    card_ptr draw_shop_card(game_ptr game);

    struct ruleset_shadowgunslingers {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(shadowgunslingers, ruleset_shadowgunslingers)

}

#endif