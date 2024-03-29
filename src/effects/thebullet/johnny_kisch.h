#ifndef __THEBULLET_JOHHNY_KISCH_H__
#define __THEBULLET_JOHHNY_KISCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_johnny_kisch : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(johnny_kisch, equip_johnny_kisch)
}

#endif