#ifndef __VALLEYOFSHADOWS_ESCAPE_H__
#define __VALLEYOFSHADOWS_ESCAPE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_escape {
        static bool can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags);
        
        bool can_play(card_ptr origin_card, player_ptr origin);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(escape, effect_escape)

    struct effect_escape2 {
        static bool can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags);
        
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(escape2, effect_escape2)
}

#endif