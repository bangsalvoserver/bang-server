#ifndef __WILDWESTSHOW_FLINT_WESTWOOD__
#define __WILDWESTSHOW_FLINT_WESTWOOD__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_flint_westwood {
        int evaluate(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(flint_westwood, effect_flint_westwood)
}

#endif
