#ifndef __GOLDRUSH_GOLDRUSH_H__
#define __GOLDRUSH_GOLDRUSH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_goldrush {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(goldrush, effect_goldrush)
}

#endif