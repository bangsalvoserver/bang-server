#ifndef __BASE_BEER_H__
#define __BASE_BEER_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct apply_beer_modifier {
            player *origin;
            nullable_ref<int> value;
        };

        struct on_play_beer {
            player *origin;
        };
    }

    struct effect_beer {
        game_string on_prompt(card *origin_card, player *target);
        void on_play(card *origin_card, player *target);
        bool can_play(card *origin_card, player *target);
    };

    DEFINE_EFFECT(beer, effect_beer)
}

#endif