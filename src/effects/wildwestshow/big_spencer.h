#ifndef __WILDWESTSHOW_BIG_SPENCER__
#define __WILDWESTSHOW_BIG_SPENCER__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_big_spencer : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(big_spencer, equip_big_spencer)

    struct equip_initial_cards : event_equip {
        int ncards;
        equip_initial_cards(int ncards): ncards{ncards} {}

        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(initial_cards, equip_initial_cards)
}

#endif