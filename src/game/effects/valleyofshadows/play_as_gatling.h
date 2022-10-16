#ifndef __VALLEYOFSHADOWS_PLAY_AS_GATLING_H__
#define __VALLEYOFSHADOWS_PLAY_AS_GATLING_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_play_as_gatling {
        void on_play(card *origin_card, player *origin, card *chosen_card);
    };
}

#endif