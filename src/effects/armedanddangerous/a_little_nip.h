#ifndef __ARMEDANDDANGEROUS_A_LITTLE_NIP_H__
#define __ARMEDANDDANGEROUS_A_LITTLE_NIP_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_a_little_nip {
        int num_cubes;
        effect_a_little_nip(int num_cubes) : num_cubes{num_cubes} {}
        
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(a_little_nip, effect_a_little_nip)
}

#endif