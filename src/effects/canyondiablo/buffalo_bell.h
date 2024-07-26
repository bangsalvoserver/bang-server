#ifndef __CANYONDIABLO_BUFFALO_BELL_H__
#define __CANYONDIABLO_BUFFALO_BELL_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_buffalo_bell {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(buffalo_bell, effect_buffalo_bell)
}

#endif