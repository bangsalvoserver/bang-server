#ifndef __BASE_BEER_H__
#define __BASE_BEER_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_beer {
        game_string on_prompt(card *origin_card, player *origin) {
            return on_prompt(origin_card, origin, origin);
        }
        game_string on_prompt(card *origin_card, player *origin, player *target);
        
        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif