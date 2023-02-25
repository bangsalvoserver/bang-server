#ifndef __CARDS_FILTER_ENUMS_H__
#define __CARDS_FILTER_ENUMS_H__

#include "utils/enums.h"

namespace banggame {

    DEFINE_ENUM_FLAGS(target_player_filter,
        (any)
        (dead)
        (self)
        (notself)
        (notsheriff)
        (range_1)
        (range_2)
        (reachable)
    )

    DEFINE_ENUM_FLAGS(target_card_filter,
        (table)
        (hand)
        (blue)
        (black)
        (clubs)
        (bang)
        (bangcard)
        (missed)
        (beer)
        (bronco)
        (cube_slot)
        (can_repeat)
        (can_target_self)
    )
}

#endif