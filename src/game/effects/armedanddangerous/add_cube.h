#ifndef __ARMEDANDDANGEROUS_ADD_CUBE_H__
#define __ARMEDANDDANGEROUS_ADD_CUBE_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_add_cube {
        int ncubes;
        effect_add_cube(int ncubes) : ncubes(std::max(ncubes, 1)) {}

        void on_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif