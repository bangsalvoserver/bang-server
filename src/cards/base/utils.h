#ifndef __BASE_UTILS_H__
#define __BASE_UTILS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_human {
        bool can_play(card *origin_card, player *origin);
    };

    struct effect_set_playing {
        void add_context(card *origin_card, player *origin, effect_context &ctx) {
            add_context(origin_card, origin, origin_card, ctx);
        }
        
        void add_context(card *origin_card, player *origin, card *target, effect_context &ctx);
    };

}

#endif