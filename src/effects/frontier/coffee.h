#ifndef __FRONTIER_COFFEE_H__
#define __FRONTIER_COFFEE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_coffee {
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(coffee, effect_coffee)
}

#endif