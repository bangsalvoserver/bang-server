#ifndef __GREATTRAINROBBERY_EVAN_BABBIT_H__
#define __GREATTRAINROBBERY_EVAN_BABBIT_H__

#include "cards/card_effect.h"

#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_evan_babbit : bot_suggestion::target_enemy {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target_player);
    };

    DEFINE_EFFECT(evan_babbit, effect_evan_babbit)
}

#endif