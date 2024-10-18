#ifndef __CANYONDIABLO_TAXMAN_H__
#define __CANYONDIABLO_TAXMAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_taxman : event_equip {
        int predraw_check_priority;
        equip_taxman(int predraw_check_priority) : predraw_check_priority{predraw_check_priority} {}
        
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(taxman, equip_taxman)
}

#endif