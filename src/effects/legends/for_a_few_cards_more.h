#ifndef __LEGENDS_FOR_A_FEW_CARDS_MORE_H__
#define __LEGENDS_FOR_A_FEW_CARDS_MORE_H__

#include "cards/card_effect.h"

#include "perform_feat.h"

namespace banggame {

    struct equip_for_a_few_cards_more : feat_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(for_a_few_cards_more, equip_for_a_few_cards_more)
}

#endif