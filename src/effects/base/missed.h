#ifndef __BASE_MISSED_H__
#define __BASE_MISSED_H__

#include "cards/card_effect.h"

namespace banggame {

    int count_missed_cards(player_ptr origin);

    struct effect_missed {
        bool can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx = {}) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(missed, effect_missed)

    struct effect_missedcard : effect_missed {
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(missedcard, effect_missedcard)

    struct effect_play_as_missed {
        bool can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(play_as_missed, effect_play_as_missed)
}

#endif