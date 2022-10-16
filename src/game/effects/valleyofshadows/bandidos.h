#ifndef __VALLEYOFSHADOWS_BANDIDOS_H__
#define __VALLEYOFSHADOWS_BANDIDOS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_bandidos {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags);
    };
}

#endif