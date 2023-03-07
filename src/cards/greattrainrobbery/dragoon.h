#ifndef __GREATTRAINROBBERY_DRAGOON_H__
#define __GREATTRAINROBBERY_DRAGOON_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_dragoon : event_equip {
        void on_enable(card *origin_card, player *origin);
    };
}

#endif