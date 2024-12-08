#ifndef __BASE_WEAPON_H__
#define __BASE_WEAPON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_weapon {
        int range;
        equip_weapon(int range): range(range) {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target, equip_flags flags);
        void on_disable(card_ptr target_card, player_ptr target, equip_flags flags);
    };

    DEFINE_EQUIP(weapon, equip_weapon)
}

#endif