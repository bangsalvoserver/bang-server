#ifndef __FRONTIER_CAYENNE_CHEE_H__
#define __FRONTIER_CAYENNE_CHEE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_cayenne_chee {

    };

    DEFINE_EQUIP(cayenne_chee, equip_cayenne_chee)

    struct effect_cayenne_chee {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(cayenne_chee, effect_cayenne_chee)
}

#endif