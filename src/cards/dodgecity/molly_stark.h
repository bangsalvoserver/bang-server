#ifndef __DODGECITY_MOLLY_STARK_H__
#define __DODGECITY_MOLLY_STARK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_molly_stark : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif