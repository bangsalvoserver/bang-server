#ifndef __LEGENDS_PEDRO_RAMIREZ_H__
#define __LEGENDS_PEDRO_RAMIREZ_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_pedro_ramirez_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(pedro_ramirez_legend, equip_pedro_ramirez_legend)

}

#endif