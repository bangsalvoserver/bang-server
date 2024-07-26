#ifndef __WILDWESTSHOW_GREYGORY_DECK__
#define __WILDWESTSHOW_GREYGORY_DECK__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_greygory_deck : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(greygory_deck, equip_greygory_deck)

    struct effect_greygory_deck {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(greygory_deck, effect_greygory_deck)
}

#endif