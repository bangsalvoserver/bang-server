#ifndef __ARMEDANDDANGEROUS_MS_ABIGAIL__
#define __ARMEDANDDANGEROUS_MS_ABIGAIL__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_ms_abigail : event_based_effect {
        bool can_escape(player *origin, card *origin_card, effect_flags flags);

        bool can_respond(card *origin_card, player *target);
        void on_play(card *origin_card, player *origin);

        void on_enable(card *origin_card, player *origin);
    };
}

#endif