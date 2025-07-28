#ifndef __ARMEDANDDANGEROUS_SQUAW_H__
#define __ARMEDANDDANGEROUS_SQUAW_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_squaw {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr discarded_card, const effect_context &ctx);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr discarded_card, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr discarded_card, effect_flags flags, const effect_context &ctx);
    };

    DEFINE_EFFECT(squaw, effect_squaw)
}

#endif