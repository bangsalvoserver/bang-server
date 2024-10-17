#ifndef __FISTFULOFCARDS_SNIPER_H__
#define __FISTFULOFCARDS_SNIPER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_sniper {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(sniper, effect_sniper)
}

#endif