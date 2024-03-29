#ifndef __BASE_MAX_USAGES_H__
#define __BASE_MAX_USAGES_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        DEFINE_STRUCT(count_usages,
            (player *, origin)
            (card *, origin_card)
            (nullable_ref<int>, usages)
        )
    }

    struct effect_max_usages {
        int max_usages;
        
        game_string get_error(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(max_usages, effect_max_usages)
}

#endif