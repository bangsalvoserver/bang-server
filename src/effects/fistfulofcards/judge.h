#ifndef __FISTFULOFCARDS_JUDGE_H__
#define __FISTFULOFCARDS_JUDGE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_judge {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(judge, equip_judge)
}

#endif