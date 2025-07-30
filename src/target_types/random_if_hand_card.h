#ifndef __TARGET_TYPE_RANDOM_IF_HAND_CARD_H__
#define __TARGET_TYPE_RANDOM_IF_HAND_CARD_H__

#include "cards/card_effect.h"

#include "card.h"

namespace banggame {

    struct targeting_random_if_hand_card : targeting_card {
        using targeting_card::targeting_card;
        
        void on_play(const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(random_if_hand_card, targeting_random_if_hand_card)
}

#endif