#ifndef __THEBULLET_CLAUS_THE_SAINT_H__
#define __THEBULLET_CLAUS_THE_SAINT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_claus_the_saint : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(claus_the_saint, equip_claus_the_saint)

    struct handler_claus_the_saint {
        bool can_play(card *origin_card, player *origin, card *target_card, player *target_player);
        void on_play(card *origin_card, player *origin, card *target_card, player *target_player);
    };

    DEFINE_MTH(claus_the_saint, handler_claus_the_saint)
}

#endif