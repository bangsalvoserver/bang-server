#ifndef __GOLDRUSH_SHOPCHOICE_H__
#define __GOLDRUSH_SHOPCHOICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_card_choice {
        bool can_play(card *origin_card, player *origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(card_choice, effect_card_choice)

    struct modifier_card_choice {
        bool valid_with_card(card *origin_card, player *origin, card *target_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
    
    DEFINE_MODIFIER(card_choice, modifier_card_choice)
}

#endif