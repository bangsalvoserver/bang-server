#ifndef __VLCATA_BEJCEK_H__
#define __VLCATA_BEJCEK_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_bejcek : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(bejcek, equip_bejcek)
}

#endif
