#ifndef __FRONTIER_SHANGO_BROTHERS_H__
#define __FRONTIER_SHANGO_BROTHERS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_shango_brothers : event_equip {
        int card_count;
        int max_cards;

        equip_shango_brothers(int card_count, int max_cards)
            : card_count{card_count}
            , max_cards{max_cards} {}

        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(shango_brothers, equip_shango_brothers)
}

#endif