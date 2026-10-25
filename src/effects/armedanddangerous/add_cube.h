#ifndef __ARMEDANDDANGEROUS_ADD_CUBE_H__
#define __ARMEDANDDANGEROUS_ADD_CUBE_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_add_cube {
        int ncubes;
        effect_add_cube(int ncubes) : ncubes{ncubes} {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;

        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target) const;
    };

    DEFINE_EFFECT(add_cube, effect_add_cube)

    struct effect_min_cubes {
        int ncubes;
        effect_min_cubes(int ncubes) : ncubes{ncubes} {}

        bool can_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(min_cubes, effect_min_cubes)
}

#endif