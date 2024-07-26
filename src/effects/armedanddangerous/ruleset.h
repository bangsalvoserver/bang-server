#ifndef __ARMEDANDDANGERSOUS_RULESET_H__
#define __ARMEDANDDANGERSOUS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct on_discard_orange_card{
            player_ptr target;
            card_ptr target_card;
        };
    }

    struct ruleset_armedanddangerous {
        void on_apply(game *game);
    };

    DEFINE_RULESET(armedanddangerous, ruleset_armedanddangerous)

}

#endif