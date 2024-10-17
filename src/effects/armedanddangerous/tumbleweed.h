#ifndef __ARMEDANDDANGEROUS_TUMBLEWEED_H__
#define __ARMEDANDDANGEROUS_TUMBLEWEED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_tumbleweed : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(tumbleweed, equip_tumbleweed)

    struct effect_tumbleweed {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin);
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(tumbleweed, effect_tumbleweed)
}

#endif