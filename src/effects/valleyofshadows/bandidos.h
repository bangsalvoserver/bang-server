#ifndef __VALLEYOFSHADOWS_BANDIDOS_H__
#define __VALLEYOFSHADOWS_BANDIDOS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_bandidos {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(bandidos, effect_bandidos)

    struct effect_bandidos2 {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(bandidos2, effect_bandidos2)

    struct effect_bandidos2_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };


    DEFINE_EFFECT(bandidos2_response, effect_bandidos2_response)
}

#endif