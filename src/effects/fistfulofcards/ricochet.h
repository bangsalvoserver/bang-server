#ifndef __FISTFULOFCARDS_RICOCHET_H__
#define __FISTFULOFCARDS_RICOCHET_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_ricochet : bot_suggestion::target_enemy_card {
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(ricochet, effect_ricochet)
}

#endif