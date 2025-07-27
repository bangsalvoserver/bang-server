#ifndef __GOLDRUSH_SELL_BEER_H__
#define __GOLDRUSH_SELL_BEER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_sell_beer {
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(sell_beer, effect_sell_beer)
}

#endif