#ifndef __FISTFULOFCARDS_VENDETTA_H__
#define __FISTFULOFCARDS_VENDETTA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vendetta : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif