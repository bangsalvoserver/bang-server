#ifndef __LEGENDS_ROSE_DOOLAN_H__
#define __LEGENDS_ROSE_DOOLAN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_rose_doolan_legend {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(rose_doolan_legend, equip_rose_doolan_legend)
}

#endif