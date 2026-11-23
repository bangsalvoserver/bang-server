#ifndef __ARMEDANDDANGEROUS_A_LITTLE_NIP_H__
#define __ARMEDANDDANGEROUS_A_LITTLE_NIP_H__

#include "cards/card_effect.h"

#include "effects/base/heal.h"

namespace banggame {

    struct effect_a_little_nip {
        int amount;
        effect_a_little_nip(int amount = 1): amount{amount} {}

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, int ncubes) const {
            return effect_heal{ncubes + amount}.on_prompt(origin_card, origin);
        }

        int evaluate(card_ptr origin_card, player_ptr origin, int ncubes) const {
            return effect_heal{ncubes + amount}.evaluate(origin_card, origin);
        }

        void on_play(card_ptr origin_card, player_ptr origin, int ncubes) const {
            effect_heal{ncubes + amount}.on_play(origin_card, origin);
        }
    };

    DEFINE_EFFECT(a_little_nip, effect_a_little_nip)
}

#endif