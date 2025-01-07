#ifndef __LEGENDS_THOUSAND_WAYS_TO_DIE_H__
#define __LEGENDS_THOUSAND_WAYS_TO_DIE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_thousand_ways_to_die {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card1, card_ptr target_card2);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card1, card_ptr target_card2);
    };

    DEFINE_MTH(thousand_ways_to_die, handler_thousand_ways_to_die)
}

#endif