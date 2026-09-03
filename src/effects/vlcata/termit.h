#ifndef __VLCATA_TERMIT_H__
#define __VLCATA_TERMIT_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_termit : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(termit, equip_termit)
}

#endif