#ifndef __FRONTIER_BALLAD_H__
#define __FRONTIER_BALLAD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ballad {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(ballad, effect_ballad)
}

#endif