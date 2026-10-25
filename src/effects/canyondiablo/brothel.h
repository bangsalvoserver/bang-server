#ifndef __CANYONDIABLO_BROTHEL_H__
#define __CANYONDIABLO_BROTHEL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_brothel {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(brothel, equip_brothel)
}

#endif