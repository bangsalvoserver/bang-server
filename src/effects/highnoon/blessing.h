#ifndef __HIGHNOON_BLESSING_H__
#define __HIGHNOON_BLESSING_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_blessing : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(blessing, equip_blessing)
}

#endif