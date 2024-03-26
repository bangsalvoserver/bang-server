#ifndef __HIGHNOON_GHOSTTOWN_H__
#define __HIGHNOON_GHOSTTOWN_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_ghosttown : event_equip  {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(ghosttown, equip_ghosttown)
}

#endif