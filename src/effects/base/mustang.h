#ifndef __BASE_MUSTANG_H__
#define __BASE_MUSTANG_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_horse {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target, equip_flags flags);
    };

    DEFINE_EQUIP(horse, equip_horse)

    struct equip_mustang : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(mustang, equip_mustang)
}

#endif