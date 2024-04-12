#ifndef __GREATTRAINROBBERY_SHADE_OCONNOR_H__
#define __GREATTRAINROBBERY_SHADE_OCONNOR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_shade_oconnor : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(shade_oconnor, equip_shade_oconnor)
}

#endif