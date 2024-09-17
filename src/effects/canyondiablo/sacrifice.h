#ifndef __CANYONDIABLO_SACRIFICE_H__
#define __CANYONDIABLO_SACRIFICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_sacrifice {
        bool can_play(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(sacrifice, effect_sacrifice)
}

#endif