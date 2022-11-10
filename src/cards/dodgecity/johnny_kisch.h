#ifndef __DODGECITY_JOHHNY_KISCH_H__
#define __DODGECITY_JOHHNY_KISCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_johnny_kisch : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif