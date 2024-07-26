#ifndef __VALLEYOFSHADOWS_POKER_H__
#define __VALLEYOFSHADOWS_POKER_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_poker {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(poker, effect_poker)
}

#endif