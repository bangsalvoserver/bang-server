#ifndef __VALLEYOFSHADOWS_BOUNTY_H__
#define __VALLEYOFSHADOWS_BOUNTY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_bounty : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif