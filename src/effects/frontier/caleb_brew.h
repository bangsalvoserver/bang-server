#ifndef __FRONTIER_CALEB_BREW_H__
#define __FRONTIER_CALEB_BREW_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_caleb_brew : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(caleb_brew, equip_caleb_brew)

    struct effect_caleb_brew {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(caleb_brew, effect_caleb_brew)
}

#endif