#ifndef __GOLDRUSH_SELL_BEER_H__
#define __GOLDRUSH_SELL_BEER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_sell_beer {
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif