#ifndef __CANYONDIABLO_GRAVEROBBER_H__
#define __CANYONDIABLO_GRAVEROBBER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_graverobber {
        void on_play(card *origin_card, player *origin);
    };
}

#endif