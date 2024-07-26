#ifndef __FISTFULOFCARDS_LASSO_H__
#define __FISTFULOFCARDS_LASSO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lasso {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(lasso, equip_lasso)
}

#endif