#ifndef __FRONTIER_TARANTULA_JUICE_H__
#define __FRONTIER_TARANTULA_JUICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_tarantula_juice {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(tarantula_juice, effect_tarantula_juice)
}

#endif