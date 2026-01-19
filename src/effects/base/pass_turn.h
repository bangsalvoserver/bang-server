#ifndef __BASE_PASS_TURN_H__
#define __BASE_PASS_TURN_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct check_pass_turn {
            using result_type = game_string;
            player_ptr origin;
        };

        struct prompt_pass_turn {
            using result_type = prompt_string;
            player_ptr origin;
        };
    }

    struct effect_pass_turn {
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(pass_turn, effect_pass_turn)
}

#endif