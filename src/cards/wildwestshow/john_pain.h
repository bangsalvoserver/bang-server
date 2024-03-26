#ifndef __WILDWESTSHOW_JOHN_PAIN__
#define __WILDWESTSHOW_JOHN_PAIN__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_john_pain : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(john_pain, equip_john_pain)
}

#endif