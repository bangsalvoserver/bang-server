#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "card_defs.h"

#include "game/request_base.h"

#include "vtables.h"

namespace banggame {

    struct event_equip {
        void on_disable(card_ptr target_card, player_ptr target);
    };

    namespace target_args {
        struct empty {};

        struct player {
            player_filter_bitset player_filter;
        };

        struct card {
            player_filter_bitset player_filter;
            card_filter_bitset card_filter;
        };
    }

}


#endif