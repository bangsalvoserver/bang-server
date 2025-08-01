#ifndef __BASE_REVEAL_CARD_H__
#define __BASE_REVEAL_CARD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_reveal_card {
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };

    DEFINE_EFFECT(reveal_card, effect_reveal_card)
}

#endif