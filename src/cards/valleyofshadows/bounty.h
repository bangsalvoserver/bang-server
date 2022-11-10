#ifndef __VALLEYOFSHADOWS_BOUNTY_H__
#define __VALLEYOFSHADOWS_BOUNTY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bounty : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif