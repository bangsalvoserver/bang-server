#ifndef __ARMEDANDDANGEROUS_A_LITTLE_NIP_H__
#define __ARMEDANDDANGEROUS_A_LITTLE_NIP_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_a_little_nip {
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx);
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(a_little_nip, effect_a_little_nip)
}

#endif