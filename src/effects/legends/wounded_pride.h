#ifndef __LEGENDS_WOUNDED_PRIDE_H__
#define __LEGENDS_WOUNDED_PRIDE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_wounded_pride : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(wounded_pride, equip_wounded_pride)
}

#endif