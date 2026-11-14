#ifndef __VLCATA_KRECEK_H__
#define __VLCATA_KRECEK_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_krecek : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(krecek, equip_krecek)
}

#endif