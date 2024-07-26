#ifndef __HIGHNOON_HANGOVER_H__
#define __HIGHNOON_HANGOVER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_hangover {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(hangover, equip_hangover)
}

#endif