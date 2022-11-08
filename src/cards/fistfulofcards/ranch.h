#ifndef __FISTFULOFCARDS_RANCH_H__
#define __FISTFULOFCARDS_RANCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_ranch : event_based_effect {
        void on_enable(card *target_card, player *target);
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif