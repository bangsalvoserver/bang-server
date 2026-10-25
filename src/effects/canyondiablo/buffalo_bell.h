#ifndef __CANYONDIABLO_BUFFALO_BELL_H__
#define __CANYONDIABLO_BUFFALO_BELL_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_buffalo_bell {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target) const;
    };

    DEFINE_EFFECT(buffalo_bell, effect_buffalo_bell)
}

#endif