#ifndef __ARMEDANDDANGEROUS_ADD_CUBE_H__
#define __ARMEDANDDANGEROUS_ADD_CUBE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_add_cube {
        int ncubes;
        effect_add_cube(int ncubes) : ncubes(std::max(ncubes, 1)) {}

        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);

        game_string on_prompt(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };

    DEFINE_EFFECT(add_cube, effect_add_cube)
}

#endif