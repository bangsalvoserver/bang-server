#ifndef __ARMEDANDDANGEROUS_TUMBLEWEED_H__
#define __ARMEDANDDANGEROUS_TUMBLEWEED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_tumbleweed : event_based_effect {
        void on_enable(card *target_card, player *target);
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif