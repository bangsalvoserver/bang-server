#ifndef __VLCATA_VUDCE_H__
#define __VLCATA_VUDCE_H__

#include "cards/card_effect.h"

namespace banggame {
    struct effect_vudce_ability {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };
    DEFINE_EFFECT(vudce_ability, effect_vudce_ability)
}

#endif