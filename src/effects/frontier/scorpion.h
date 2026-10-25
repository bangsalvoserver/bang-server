#ifndef __FRONTIER_SCORPION_H__
#define __FRONTIER_SCORPION_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_scorpion : event_equip {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(scorpion, equip_scorpion)
}

#endif