#ifndef __GREATTRAINROBBERY_DRAGOON_H__
#define __GREATTRAINROBBERY_DRAGOON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_dragoon {
        void on_enable(card_ptr origin_card, player_ptr origin);
        void on_disable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(dragoon, equip_dragoon)
}

#endif