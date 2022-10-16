#ifndef __VALLEYOFSHADOWS_SAVED_H__
#define __VALLEYOFSHADOWS_SAVED_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_saved {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif