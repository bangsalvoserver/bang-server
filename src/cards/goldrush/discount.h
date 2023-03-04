#ifndef __GOLDRUSH_DISCOUNT_H__
#define __GOLDRUSH_DISCOUNT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_discount {
        bool valid_with_card(card *origin_card, player *origin, card *target_card);

        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif