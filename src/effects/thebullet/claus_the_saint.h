#ifndef __THEBULLET_CLAUS_THE_SAINT_H__
#define __THEBULLET_CLAUS_THE_SAINT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_claus_the_saint : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(claus_the_saint, equip_claus_the_saint)

    struct effect_claus_the_saint {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(claus_the_saint, effect_claus_the_saint)

    struct handler_claus_the_saint {
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target_player);
    };

    DEFINE_MTH(claus_the_saint, handler_claus_the_saint)
}

#endif