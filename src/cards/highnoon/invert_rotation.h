#ifndef __HIGHNOON_INVERT_ROTATION_H__
#define __HIGHNOON_INVERT_ROTATION_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_invert_rotation  {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif