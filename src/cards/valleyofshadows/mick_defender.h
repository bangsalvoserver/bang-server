#ifndef __VALLEYOFSHADOWS_MICK_DEFENDER_H__
#define __VALLEYOFSHADOWS_MICK_DEFENDER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_mick_defender : event_equip {
        void on_enable(card *origin_card, player *origin);
    };
}

#endif