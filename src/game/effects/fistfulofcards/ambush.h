#ifndef __FISTFULOFCARDS_AMBUSH_H__
#define __FISTFULOFCARDS_AMBUSH_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_ambush {
        void on_enable(card *target_card, player *target);
    };
}

#endif