#ifndef __FISTFULOFCARDS_FISTFULOFCARDS_H__
#define __FISTFULOFCARDS_FISTFULOFCARDS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_fistfulofcards : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif