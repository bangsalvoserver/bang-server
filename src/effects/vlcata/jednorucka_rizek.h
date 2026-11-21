#ifndef __VLCATA_JEDNORUCKA_RIZEK_H__
#define __VLCATA_JEDNORUCKA_RIZEK_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_jednorucka_rizek : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(jednorucka_rizek, equip_jednorucka_rizek)
}

#endif