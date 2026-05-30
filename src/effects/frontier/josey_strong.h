#ifndef __FRONTIER_JOSEY_STRONG_H__
#define __FRONTIER_JOSEY_STRONG_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_josey_strong {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(josey_strong, effect_josey_strong)
}

#endif