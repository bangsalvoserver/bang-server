#ifndef __VLCATA_MIKI_H__
#define __VLCATA_MIKI_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_miki : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(miki, equip_miki)
}

#endif