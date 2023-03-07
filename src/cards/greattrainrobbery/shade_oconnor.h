#ifndef __GREATTRAINROBBERY_SHADE_OCONNOR_H__
#define __GREATTRAINROBBERY_SHADE_OCONNOR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_shade_oconnor {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct equip_shade_oconnor : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif