#ifndef __BASE_MAX_USAGES_H__
#define __BASE_MAX_USAGES_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct count_usages {
            player_ptr origin;
            card_ptr origin_card;
            nullable_ref<int> usages;
        };
    }

    struct effect_max_usages {
        int max_usages;
        
        game_string get_error(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(max_usages, effect_max_usages)
}

#endif