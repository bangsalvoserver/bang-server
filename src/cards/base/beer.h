#ifndef __BASE_BEER_H__
#define __BASE_BEER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_beer {
        game_string on_prompt(card *origin_card, player *target);
        void on_play(card *origin_card, player *target);
    };

    struct effect_beer_response : effect_beer {
        bool can_respond(card *origin_card, player *target);
    };
}

#endif