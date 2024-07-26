#ifndef __GREATTRAINROBBERY_MAP_H__
#define __GREATTRAINROBBERY_MAP_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_map : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(map, equip_map)
}

#endif