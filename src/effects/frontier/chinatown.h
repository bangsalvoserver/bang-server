#ifndef __FRONTIER_CHINATOWN_H__
#define __FRONTIER_CHINATOWN_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_chinatown {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(chinatown, effect_chinatown)
}

#endif