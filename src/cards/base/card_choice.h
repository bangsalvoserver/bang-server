#ifndef __GOLDRUSH_SHOPCHOICE_H__
#define __GOLDRUSH_SHOPCHOICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_card_choice {
        bool can_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    struct modifier_card_choice {
        bool valid_with_card(card *origin_card, player *origin, card *target_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif