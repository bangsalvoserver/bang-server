#ifndef __FISTFULOFCARDS_RANCH_H__
#define __FISTFULOFCARDS_RANCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ranch : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(ranch, equip_ranch)

    struct effect_ranch {
        bool can_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(ranch, effect_ranch)

    struct handler_ranch {
        void on_play(card *origin_card, player *origin, const card_list &target_cards);
    };

    DEFINE_MTH(ranch, handler_ranch)
}

#endif