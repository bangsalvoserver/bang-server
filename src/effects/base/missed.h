#ifndef __BASE_MISSED_H__
#define __BASE_MISSED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_missed {
        bool can_play(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(missed, effect_missed)

    struct effect_missedcard : effect_missed {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(missedcard, effect_missedcard)

    struct handler_play_as_missed {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };

    DEFINE_MTH(play_as_missed, handler_play_as_missed)
}

#endif