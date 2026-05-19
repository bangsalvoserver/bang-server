#ifndef __FRONTIER_HAWKEN_H__
#define __FRONTIER_HAWKEN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_hawken : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(hawken, equip_hawken)

    struct effect_hawken {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(hawken, effect_hawken)
}

#endif