#ifndef __ARMEDANDDANGEROUS_FLINTLOCK_H__
#define __ARMEDANDDANGEROUS_FLINTLOCK_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_flintlock {
        void on_play(card *origin_card, player *origin, player *target, opt_tagged_value<target_type::none> paid_cubes);
    };
}

#endif