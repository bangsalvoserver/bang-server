#ifndef __CANYONDIABLO_SACRIFICE_H__
#define __CANYONDIABLO_SACRIFICE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_sacrifice {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif