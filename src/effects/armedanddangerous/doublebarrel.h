#ifndef __ARMEDANDDANGEROUS_DOUBLEBARREL_H__
#define __ARMEDANDDANGEROUS_DOUBLEBARREL_H__

#include "cards/card_effect.h"
#include "effects/base/bang.h"

namespace banggame {

    struct effect_doublebarrel {
        void on_play(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(doublebarrel, effect_doublebarrel)
}

#endif