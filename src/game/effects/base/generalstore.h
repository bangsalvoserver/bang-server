#ifndef __BASE_GENERALSTORE_H__
#define __BASE_GENERALSTORE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_generalstore {
        void on_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif