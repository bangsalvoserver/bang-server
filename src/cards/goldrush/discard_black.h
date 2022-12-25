#ifndef __GOLDRUSH_DISCARD_BLACK_H__
#define __GOLDRUSH_DISCARD_BLACK_H__

#include "cards/card_effect.h"
#include "cards/base/steal_destroy.h"

namespace banggame {

    struct effect_discard_black : prompt_target_self {
        game_string verify(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif