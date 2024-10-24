#ifndef __VALLEYOFSHADOWS_MICK_DEFENDER_H__
#define __VALLEYOFSHADOWS_MICK_DEFENDER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_mick_defender : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(mick_defender, equip_mick_defender)

    struct equip_mick_defender2 : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(mick_defender2, equip_mick_defender2)
}

#endif