#ifndef __FISTFULOFCARDS_RANCH_H__
#define __FISTFULOFCARDS_RANCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ranch : event_equip {
        void on_enable(card *target_card, player *target);
    };

    struct effect_ranch {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif