#ifndef __FISTFULOFCARDS_RANCH_H__
#define __FISTFULOFCARDS_RANCH_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_ranch : event_equip {
        void on_enable(card *target_card, player *target);
    };

    struct handler_ranch {
        bool can_play(card *origin_card, player *origin, const serial::card_list &target_cards);
        void on_play(card *origin_card, player *origin, const serial::card_list &target_cards);
    };

    DEFINE_MTH(ranch, handler_ranch)
}

#endif