#ifndef __FRONTIER_ALEXANDER_NOON_H__
#define __FRONTIER_ALEXANDER_NOON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_alexander_noon : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(alexander_noon, equip_alexander_noon)

    struct effect_alexander_noon {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(alexander_noon, effect_alexander_noon)
}

#endif