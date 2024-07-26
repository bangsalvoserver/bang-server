#ifndef __GOLDRUSH_DISCOUNT_H__
#define __GOLDRUSH_DISCOUNT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_discount {
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card);

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(discount, modifier_discount)
}

#endif