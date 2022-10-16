#ifndef __VALLEYOFSHADOWS_BACKFIRE_H__
#define __VALLEYOFSHADOWS_BACKFIRE_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_backfire {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif