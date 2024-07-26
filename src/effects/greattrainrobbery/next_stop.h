#ifndef __GREATTRAINROBBERY_NEXT_STOP_H__
#define __GREATTRAINROBBERY_NEXT_STOP_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_next_stop {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(next_stop, effect_next_stop)
}

#endif