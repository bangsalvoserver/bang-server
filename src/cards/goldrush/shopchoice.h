#ifndef __GOLDRUSH_SHOPCHOICE_H__
#define __GOLDRUSH_SHOPCHOICE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_shopchoice {
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx);
    };

    struct modifier_shopchoice {
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };
}

#endif