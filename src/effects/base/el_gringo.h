#ifndef __BASE_EL_GRINGO_H__
#define __BASE_EL_GRINGO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_el_gringo : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(el_gringo, equip_el_gringo)
}

#endif