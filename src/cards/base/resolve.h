#ifndef __BASE_RESOLVE_H__
#define __BASE_RESOLVE_H__

#include "cards/card_effect.h"

namespace banggame {
    struct effect_resolve {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif