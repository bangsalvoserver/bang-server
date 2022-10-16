#ifndef __FISTFULOFCARDS_DEADMAN_H__
#define __FISTFULOFCARDS_DEADMAN_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_deadman : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif