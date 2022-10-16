#ifndef __ARMEDANDDANGEROUS_BIGFIFTY_H__
#define __ARMEDANDDANGEROUS_BIGFIFTY_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_bigfifty {
        void on_play(card *origin_card, player *origin);
    };
}

#endif