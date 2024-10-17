#ifndef __FISTFULOFCARDS_RICOCHET_H__
#define __FISTFULOFCARDS_RICOCHET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ricochet {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(ricochet, effect_ricochet)
}

#endif