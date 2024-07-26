#ifndef __HIGHNOON_TRAINARRIVAL_H__
#define __HIGHNOON_TRAINARRIVAL_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_trainarrival : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(trainarrival, equip_trainarrival)
}

#endif