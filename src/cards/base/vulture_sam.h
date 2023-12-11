#ifndef __BASE_VULTURE_SAM_H__
#define __BASE_VULTURE_SAM_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        DEFINE_STRUCT(check_card_taker,
            (player *, target)
            (int, type)
            (nullable_ref<card *>, value)
        )
    }
    
    struct equip_vulture_sam : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif