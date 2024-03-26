#ifndef __HIGHNOON_SERMON_H__
#define __HIGHNOON_SERMON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_sermon {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(sermon, equip_sermon)
}

#endif