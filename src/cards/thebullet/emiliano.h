#ifndef __THEBULLET_EMILIANO_H__
#define __THEBULLET_EMILIANO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_emiliano : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif