#ifndef __GOLDRUSH_RUM_H__
#define __GOLDRUSH_RUM_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_rum {
        void on_play(card *origin_card, player *origin);
    };
}

#endif