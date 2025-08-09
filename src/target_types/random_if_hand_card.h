#ifndef __TARGET_TYPE_RANDOM_IF_HAND_CARD_H__
#define __TARGET_TYPE_RANDOM_IF_HAND_CARD_H__

#include "card.h"

namespace banggame {

    struct targeting_random_if_hand_card : targeting_card {
        using targeting_card::targeting_card;
        
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
            if (target->pocket == pocket_type::player_hand) {
                target = target->owner->random_hand_card();
            }
            targeting_card::on_play(origin_card, origin, effect, ctx, target);
        }
    };

    DEFINE_TARGETING(random_if_hand_card, targeting_random_if_hand_card)
}

#endif