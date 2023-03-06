#ifndef __GREATTRAINROBBERY_LOUNGE_CAR_H__
#define __GREATTRAINROBBERY_LOUNGE_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_lounge_car {
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct handler_lounge_car {
        game_string get_error(card *origin_card, player *origin, card *target_card, player *target);
        void on_play(card *origin_card, player *origin, card *target_card, player *target);  
    };
}

#endif