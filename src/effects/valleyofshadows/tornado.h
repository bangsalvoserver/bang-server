#ifndef __VALLEYOFSHADOWS_TORNADO_H__
#define __VALLEYOFSHADOWS_TORNADO_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_tornado {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(tornado, effect_tornado)
    
    struct effect_tornado2 {
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(tornado2, effect_tornado2)

    struct effect_tornado2_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(tornado2_response, effect_tornado2_response)
}

#endif