#ifndef __GOLDRUSH_WANTED_H__
#define __GOLDRUSH_WANTED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wanted : event_equip {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(wanted, equip_wanted)
}

#endif