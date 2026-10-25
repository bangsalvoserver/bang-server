#ifndef __VALLEYOFSHADOWS_SAVED_H__
#define __VALLEYOFSHADOWS_SAVED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_saved {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(saved, effect_saved)

    struct effect_saved2 {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(saved2, effect_saved2)
}

#endif