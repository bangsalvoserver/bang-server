#ifndef __GREATTRAINROBBERY_EVAN_BABBIT_H__
#define __GREATTRAINROBBERY_EVAN_BABBIT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_evan_babbit {
        bool can_play(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target_player);
    };

    DEFINE_EFFECT(evan_babbit, effect_evan_babbit)
}

#endif