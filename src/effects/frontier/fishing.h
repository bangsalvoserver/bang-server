#ifndef __FRONTIER_FISHING_H__
#define __FRONTIER_FISHING_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_fishing {
        card_suit suit;
        effect_fishing(card_suit suit): suit{suit} {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
    };

    DEFINE_EFFECT(fishing, effect_fishing)
}

#endif