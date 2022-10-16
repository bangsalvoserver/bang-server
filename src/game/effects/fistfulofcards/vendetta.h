#ifndef __FISTFULOFCARDS_VENDETTA_H__
#define __FISTFULOFCARDS_VENDETTA_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_vendetta : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif