#ifndef __FRONTIER_TRAP_H__
#define __FRONTIER_TRAP_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_trap : event_equip {
        game_string on_prompt(card_ptr target_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(trap, equip_trap)

    struct effect_trap {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(trap, effect_trap)
}

#endif