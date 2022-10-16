#ifndef __ARMEDANDDANGEROUS_BUNTLINESPECIAL_H__
#define __ARMEDANDDANGEROUS_BUNTLINESPECIAL_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_buntlinespecial {
        void on_play(card *origin_card, player *origin);
    };
}

#endif