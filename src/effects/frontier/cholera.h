#ifndef __FRONTIER_CHOLERA_H__
#define __FRONTIER_CHOLERA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_cholera : event_equip {
        game_string on_prompt(card_ptr target_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(cholera, equip_cholera)
}

#endif