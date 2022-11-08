#ifndef __CANYONDIABLO_LASTWILL_H__
#define __CANYONDIABLO_LASTWILL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_lastwill : event_based_effect {
        void on_enable(card *origin_card, player *origin);
        
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin) {}
    };

    struct handler_lastwill {
        void on_play(card *origin_card, player *origin, const target_list &targets);
    };
}

#endif