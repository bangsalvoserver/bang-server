#ifndef __VALLEYOFSHADOWS_FANNING_H__
#define __VALLEYOFSHADOWS_FANNING_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_fanning {
        game_string get_error(card *origin_card, player *origin, player *target1, player *target2);
        bool on_check_target(card *origin_card, player *origin, player *target1, player *target2);
        void on_play(card *origin_card, player *origin, player *target1, player *target2);
    };
}

#endif