#ifndef __BASE_GIFT_CARD_H__
#define __BASE_GIFT_CARD_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_gift_card {
        bool used;
        bool forced_response;

        handler_gift_card(bool used = false, bool forced_response = false)
            : used{used}
            , forced_response{forced_response} {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, player_ptr target);
    };

    DEFINE_MTH(gift_card, handler_gift_card)
}

#endif