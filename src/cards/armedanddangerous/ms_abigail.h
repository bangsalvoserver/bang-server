#ifndef __ARMEDANDDANGEROUS_MS_ABIGAIL__
#define __ARMEDANDDANGEROUS_MS_ABIGAIL__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_ms_abigail : event_equip {
        void on_enable(card *origin_card, player *origin);
    };

    struct effect_ms_abigail {
        bool can_respond(card *origin_card, player *target);
        void on_play(card *origin_card, player *origin);
    };
}

#endif