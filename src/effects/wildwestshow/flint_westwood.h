#ifndef __WILDWESTSHOW_FLINT_WESTWOOD__
#define __WILDWESTSHOW_FLINT_WESTWOOD__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_flint_westwood {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    };

    DEFINE_MTH(flint_westwood, handler_flint_westwood)
}

#endif