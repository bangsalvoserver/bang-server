#ifndef __BASE_BEER_H__
#define __BASE_BEER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_beer {
        bool is_response;
        effect_beer(int value) : is_response(value) {}
        
        game_string on_prompt(card *origin_card, player *target);
        void on_play(card *origin_card, player *target);
        bool can_play(card *origin_card, player *target);
    };
}

#endif