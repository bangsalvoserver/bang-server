#ifndef __VLCATA_TUCTUC_H__
#define __VLCATA_TUCTUC_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_tuctuc : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

}

#endif