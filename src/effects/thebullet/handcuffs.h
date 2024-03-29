#ifndef __THEBULLET_HANDCUFFS_H__
#define __THEBULLET_HANDCUFFS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_handcuffs : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(handcuffs, equip_handcuffs)
}

#endif