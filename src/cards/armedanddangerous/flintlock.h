#ifndef __ARMEDANDDANGEROUS_FLINTLOCK_H__
#define __ARMEDANDDANGEROUS_FLINTLOCK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_flintlock {
        void on_play(card *origin_card, player *origin, player *target, bool paid_cubes);
    };
}

#endif