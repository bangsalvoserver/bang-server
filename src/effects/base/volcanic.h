#ifndef __BASE_VOLCANIC_H__
#define __BASE_VOLCANIC_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_volcanic {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(volcanic, equip_volcanic)
}

#endif