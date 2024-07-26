#ifndef __GREATTRAINROBBERY_STRONGBOX_H__
#define __GREATTRAINROBBERY_STRONGBOX_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_strongbox : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(strongbox, equip_strongbox)
}

#endif