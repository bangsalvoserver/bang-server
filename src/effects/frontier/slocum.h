#ifndef __FRONTIER_SLOCUM_H__
#define __FRONTIER_SLOCUM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_slocum : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(slocum, equip_slocum)
}

#endif