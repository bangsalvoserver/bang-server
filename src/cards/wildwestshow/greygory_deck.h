#ifndef __WILDWESTSHOW_GREYGORY_DECK__
#define __WILDWESTSHOW_GREYGORY_DECK__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_greygory_deck {
        void on_enable(card *target_card, player *target);
    };

    struct effect_greygory_deck {
        void on_play(card *origin_card, player *origin);
    };
}

#endif