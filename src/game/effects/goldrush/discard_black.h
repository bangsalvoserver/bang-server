#ifndef __GOLDRUSH_DISCARD_BLACK_H__
#define __GOLDRUSH_DISCARD_BLACK_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_discard_black {
        game_string verify(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif