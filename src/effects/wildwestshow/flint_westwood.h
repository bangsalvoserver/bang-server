#ifndef __WILDWESTSHOW_FLINT_WESTWOOD__
#define __WILDWESTSHOW_FLINT_WESTWOOD__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct handler_flint_westwood {
        bool on_check_target(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
            return bot_suggestion::target_enemy{}.on_check_target(origin_card, origin, target_card);
        }
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    };

    DEFINE_MTH(flint_westwood, handler_flint_westwood)
}

#endif