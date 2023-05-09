#ifndef __GREATTRAINROBBERY_EXPRESS_CAR_H__
#define __GREATTRAINROBBERY_EXPRESS_CAR_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_express_car {
        game_string get_error(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif