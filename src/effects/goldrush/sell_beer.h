#ifndef __GOLDRUSH_SELL_BEER_H__
#define __GOLDRUSH_SELL_BEER_H__

#include "cards/card_effect.h"
#include "effects/base/utils.h"

namespace banggame {

    struct effect_sell_beer : effect_set_playing {
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(sell_beer, effect_sell_beer)
}

#endif