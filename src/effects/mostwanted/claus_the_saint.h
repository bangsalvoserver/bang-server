#ifndef __MOSTWANTED_CLAUS_THE_SAINT_H__
#define __MOSTWANTED_CLAUS_THE_SAINT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_claus_the_saint : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(claus_the_saint, equip_claus_the_saint)

    struct effect_claus_the_saint_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(claus_the_saint_response, effect_claus_the_saint_response)
}

#endif