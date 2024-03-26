#ifndef __WILDWESTSHOW_TEREN_KILL__
#define __WILDWESTSHOW_TEREN_KILL__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_teren_kill : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    DEFINE_EQUIP(teren_kill, equip_teren_kill)
}

#endif