#ifndef __ARMEDANDDANGEROUS_SQUAW_H__
#define __ARMEDANDDANGEROUS_SQUAW_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_squaw {
        void on_play(card *origin_card, player *origin, card *discarded_card, opt_tagged_value<target_type::none> paid_cubes);
    };
}

#endif