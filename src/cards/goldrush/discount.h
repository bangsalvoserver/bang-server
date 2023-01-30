#ifndef __GOLDRUSH_DISCOUNT_H__
#define __GOLDRUSH_DISCOUNT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_discount {
        void add_context(card *origin_card, player *origin, card *playing_card, effect_context &ctx);
    };
}

#endif