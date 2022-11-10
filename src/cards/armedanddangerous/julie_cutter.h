#ifndef __ARMEDANDDANGEROUS_JULIE_CUTTER_H__
#define __ARMEDANDDANGEROUS_JULIE_CUTTER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_julie_cutter : event_equip {
        void on_enable(card *target_card, player *target);
    };

}

#endif