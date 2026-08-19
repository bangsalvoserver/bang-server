#ifndef __VLCATA_HELENA_H__
#define __VLCATA_HELENA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct effect_helena_ability {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };
    DEFINE_EFFECT(helena_ability, effect_helena_ability)
}

#endif