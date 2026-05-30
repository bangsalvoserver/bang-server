#ifndef __FRONTIER_JOSIAH_TUNG_H__
#define __FRONTIER_JOSIAH_TUNG_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_josiah_tung : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(josiah_tung, equip_josiah_tung)
}

#endif