#ifndef __BASE_GIFT_CARD_H__
#define __BASE_GIFT_CARD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_gift_card {
        bool used;
        effect_gift_card(bool used = false): used{used} {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
    };

    DEFINE_EFFECT(gift_card, effect_gift_card)
}

#endif