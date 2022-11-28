#ifndef __GOLDRUSH_JOSH_MCCLOUD_H__
#define __GOLDRUSH_JOSH_MCCLOUD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_forced_play {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_josh_mccloud {
        void on_play(card *origin_card, player *origin);
    };
}

#endif