#ifndef __VALLEYOFSHADOWS_SNAKE_H__
#define __VALLEYOFSHADOWS_SNAKE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_snake : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif