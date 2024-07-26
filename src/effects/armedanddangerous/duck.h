#ifndef __ARMEDANDDANGEROUS_DUCK_H__
#define __ARMEDANDDANGEROUS_DUCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_duck {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(duck, effect_duck)
}

#endif