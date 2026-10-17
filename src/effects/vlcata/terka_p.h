#ifndef __VLCATA_TERKA_P_H__
#define __VLCATA_TERKA_P_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_terka_p_bang : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(terka_p_bang, equip_terka_p_bang)
}

#endif