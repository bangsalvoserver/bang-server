#ifndef __BASE_DYNAMITE_H__
#define __BASE_DYNAMITE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_dynamite : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif