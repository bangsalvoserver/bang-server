#ifndef __FISTFULOFCARDS_SNIPER_H__
#define __FISTFULOFCARDS_SNIPER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_sniper {
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif