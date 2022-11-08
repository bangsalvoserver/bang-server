#ifndef __BASE_VULTURE_SAM_H__
#define __BASE_VULTURE_SAM_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_vulture_sam : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif