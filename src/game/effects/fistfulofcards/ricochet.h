#ifndef __FISTFULOFCARDS_RICOCHET_H__
#define __FISTFULOFCARDS_RICOCHET_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_ricochet {
        void on_play(card *origin_card, player *origin, card *target);
    };
}

#endif