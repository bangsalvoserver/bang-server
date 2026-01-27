#ifndef __BASE_BEER_H__
#define __BASE_BEER_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct apply_beer_modifier {
            using result_type = int;
            player_ptr origin;
        };

        struct on_play_beer {
            player_ptr origin;
            bool is_sold;
        };
    }

    struct effect_beer {
        game_string on_prompt(card_ptr origin_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr target);
        bool can_play(card_ptr origin_card, player_ptr target);
    };

    DEFINE_EFFECT(beer, effect_beer)
}

#endif