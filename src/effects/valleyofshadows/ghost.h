#ifndef __VALLEYOFSHADOWS_GHOST_H__
#define __VALLEYOFSHADOWS_GHOST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ghost {
        struct nodisable {};
        
        player_flag flag;
        equip_ghost(player_flag flag) : flag{flag} {}
        
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(ghost, equip_ghost)
}

#endif