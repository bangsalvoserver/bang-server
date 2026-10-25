#ifndef __BASE_BARREL_H__
#define __BASE_BARREL_H__

#include "missed.h"

namespace banggame {
    
    struct effect_barrel {
        bool can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr target) const;
    };

    DEFINE_EFFECT(barrel, effect_barrel)
}

#endif