#ifndef __WILDWESTSHOW_GREYGORY_DECK__
#define __WILDWESTSHOW_GREYGORY_DECK__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_greygory_deck : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(greygory_deck, equip_greygory_deck)

    struct effect_greygory_deck {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(greygory_deck, effect_greygory_deck)
}

#endif