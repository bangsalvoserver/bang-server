#ifndef __CANYONDIABLO_MIRAGE_H__
#define __CANYONDIABLO_MIRAGE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_mirage {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif