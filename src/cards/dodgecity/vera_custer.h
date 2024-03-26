#ifndef __DODGECITY_VERA_CUSTER_H__
#define __DODGECITY_VERA_CUSTER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vera_custer : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(vera_custer, equip_vera_custer)
}

#endif