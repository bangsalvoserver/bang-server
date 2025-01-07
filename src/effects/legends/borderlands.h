#ifndef __LEGENDS_BORDERLANDS_H__
#define __LEGENDS_BORDERLANDS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_borderlands {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(borderlands, effect_borderlands)
}

#endif