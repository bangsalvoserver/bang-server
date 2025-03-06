#ifndef __ARMEDANDDANGEROUS_ADD_CUBE_H__
#define __ARMEDANDDANGEROUS_ADD_CUBE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_add_cube {
        int ncubes;
        effect_add_cube(int ncubes) : ncubes{ncubes} {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);

        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(add_cube, effect_add_cube)
}

#endif