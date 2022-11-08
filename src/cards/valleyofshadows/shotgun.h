#ifndef __VALLEYOFSHADOWS_SHOTGUN_H__
#define __VALLEYOFSHADOWS_SHOTGUN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_shotgun : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif