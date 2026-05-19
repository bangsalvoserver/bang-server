#ifndef __FRONTIER_STAMPEDE_H__
#define __FRONTIER_STAMPEDE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_stampede {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(stampede, effect_stampede)
}

#endif