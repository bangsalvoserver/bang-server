#ifndef __BASE_SLAB_THE_KILLER_H__
#define __BASE_SLAB_THE_KILLER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_slab_the_killer : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif