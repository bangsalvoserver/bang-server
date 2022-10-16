#ifndef __WILDWESTSHOW_GREYGORY_DECK__
#define __WILDWESTSHOW_GREYGORY_DECK__

#include "../card_effect.h"

namespace banggame {

    struct effect_greygory_deck : event_based_effect {
        void on_equip(card *target_card, player *target);
        void on_play(card *origin_card, player *origin);
    };
}

#endif