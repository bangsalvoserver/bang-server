#ifndef __GOLDRUSH_SHOPCHOICE_H__
#define __GOLDRUSH_SHOPCHOICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_shopchoice {
        bool can_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    struct modifier_shopchoice {
        bool valid_with_modifier(card *origin_card, player *origin, card *target_card) {
            return false;
        }

        bool valid_with_card(card *origin_card, player *origin, card *target_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif