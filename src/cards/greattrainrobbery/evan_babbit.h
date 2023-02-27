#ifndef __GREATTRAINROBBERY_EVAN_BABBIT_H__
#define __GREATTRAINROBBERY_EVAN_BABBIT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_evan_babbit {
        game_string get_error(card *origin_card, player *origin, card *target_card, player *target_player);
        void on_play(card *origin_card, player *origin, card *target_card, player *target_player);
    };
}

#endif