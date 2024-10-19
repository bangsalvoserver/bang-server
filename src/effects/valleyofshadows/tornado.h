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
    };

    DEFINE_EFFECT(tornado2_response, effect_tornado2_response)

    struct handler_tornado2_response {
        void on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards);
    };

    DEFINE_MTH(tornado2_response, handler_tornado2_response)
}

#endif