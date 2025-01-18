#ifndef __CANYONDIABLO_TAXMAN_H__
#define __CANYONDIABLO_TAXMAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_taxman : event_equip {
        int priority;
        equip_taxman(int priority) : priority{priority} {}
        
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(taxman, equip_taxman)
}

#endif