#ifndef __VALLEYOFSHADOWS_LEMONADE_JIM_H__
#define __VALLEYOFSHADOWS_LEMONADE_JIM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lemonade_jim : event_equip {
        void on_enable(card *target_card, player *target);
    };

    struct effect_lemonade_jim {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif