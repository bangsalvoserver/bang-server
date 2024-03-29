#ifndef __BASE_PASS_TURN_H__
#define __BASE_PASS_TURN_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        DEFINE_STRUCT(check_pass_turn,
            (player *, origin)
            (nullable_ref<game_string>, out_error)
        )
    }

    struct effect_pass_turn {
        game_string get_error(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(pass_turn, effect_pass_turn)
}

#endif