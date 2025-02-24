#ifndef __BASE_GIFT_CARD_H__
#define __BASE_GIFT_CARD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_gift_card {
        bool used;
        handler_gift_card(bool used = false): used{used} {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
    };

    DEFINE_MTH(gift_card, handler_gift_card)

    struct handler_gift_used_card : handler_gift_card {
        handler_gift_used_card(): handler_gift_card{true} {}
    };

    DEFINE_MTH(gift_used_card, handler_gift_used_card)

    struct handler_gift_card_selection : handler_gift_card {
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
    };

    DEFINE_MTH(gift_card_selection, handler_gift_card_selection)
}

#endif