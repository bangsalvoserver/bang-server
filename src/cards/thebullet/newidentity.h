#ifndef __THEBULLET_NEWIDENTITY_H__
#define __THEBULLET_NEWIDENTITY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_newidentity : event_equip {
        void on_enable(card *target_card, player *target);
    };

}

#endif