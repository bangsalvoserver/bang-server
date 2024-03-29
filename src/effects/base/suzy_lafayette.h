#ifndef __BASE_SUZY_LAFAYETTE_H__
#define __BASE_SUZY_LAFAYETTE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_suzy_lafayette : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(suzy_lafayette, equip_suzy_lafayette)
}

#endif