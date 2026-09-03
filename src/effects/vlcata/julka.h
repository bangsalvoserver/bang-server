#ifndef __VLCATA_JULKA_H__
#define __VLCATA_JULKA_H__

#include "cards/card_effect.h"

namespace banggame {

    // Handler for Julka: loses 1 HP at the end of her turn if HP > 3
    struct equip_julka : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(julka, equip_julka)

}

#endif