#ifndef __BASE_VULTURE_SAM_H__
#define __BASE_VULTURE_SAM_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_vulture_sam : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif