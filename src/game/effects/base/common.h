#ifndef __BASE_COMMON_H__
#define __BASE_COMMON_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_play_card_action {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_max_usages {
        int max_usages;
        
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_pass_turn {
        game_string verify(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_resolve {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif