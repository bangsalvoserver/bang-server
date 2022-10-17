#ifndef __VALLEYOFSHADOWS_MICK_DEFENDER_H__
#define __VALLEYOFSHADOWS_MICK_DEFENDER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_mick_defender : event_based_effect {
        void on_enable(card *origin_card, player *origin);
    };
}

#endif