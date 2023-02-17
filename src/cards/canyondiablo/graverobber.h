#ifndef __CANYONDIABLO_GRAVEROBBER_H__
#define __CANYONDIABLO_GRAVEROBBER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_graverobber {
        int state;
        effect_graverobber(int state) : state(state) {}
        
        void on_play(card *origin_card, player *origin);
    };
}

#endif