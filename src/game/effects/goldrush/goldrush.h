#ifndef __GOLDRUSH_GOLDRUSH_H__
#define __GOLDRUSH_GOLDRUSH_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_goldrush {
        void on_play(card *origin_card, player *origin);
    };
}

#endif