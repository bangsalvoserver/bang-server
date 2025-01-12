#ifndef __MOSTWANTED_EMILIANO_H__
#define __MOSTWANTED_EMILIANO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_emiliano : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(emiliano, equip_emiliano)
}

#endif