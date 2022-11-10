#ifndef __FISTFULOFCARDS_AMBUSH_H__
#define __FISTFULOFCARDS_AMBUSH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ambush {
        void on_enable(card *target_card, player *target);
    };
}

#endif