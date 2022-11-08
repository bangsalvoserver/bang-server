#ifndef __FISTFULOFCARDS_ABANDONEDMINE_H__
#define __FISTFULOFCARDS_ABANDONEDMINE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_abandonedmine {
        void on_enable(card *target_card, player *target);
    };
}

#endif