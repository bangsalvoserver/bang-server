#ifndef __WILDWESTSHOW_FLINT_WESTWOOD__
#define __WILDWESTSHOW_FLINT_WESTWOOD__

#include "../card_effect.h"

namespace banggame {

    struct handler_flint_westwood {
        void on_play(card *origin_card, player *origin, card *chosen_card, card *target_card);
    };
}

#endif