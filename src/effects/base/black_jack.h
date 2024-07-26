#ifndef __BASE_BLACK_JACK_H__
#define __BASE_BLACK_JACK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_black_jack : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(black_jack, equip_black_jack)
}

#endif