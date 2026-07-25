#ifndef __BASE_DUEL_H__
#define __BASE_DUEL_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_duel {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
    };

    DEFINE_EFFECT(duel, effect_duel)
}

#endif