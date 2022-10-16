#ifndef __DODGECITY_VERA_CUSTER_H__
#define __DODGECITY_VERA_CUSTER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_vera_custer : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif