// libor.h
#ifndef __VLCATA_LIBOR_H__
#define __VLCATA_LIBOR_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_libor : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(libor, equip_libor)
}
#endif