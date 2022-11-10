#ifndef __BASE_BLACK_JACK_H__
#define __BASE_BLACK_JACK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_black_jack : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif