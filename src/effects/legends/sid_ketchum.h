#ifndef __LEGENDS_SID_KETCHUM_H__
#define __LEGENDS_SID_KETCHUM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_sid_ketchum_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(sid_ketchum_legend, equip_sid_ketchum_legend)

}

#endif