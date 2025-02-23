#ifndef __VALLEYOFSHADOWS_COLORADO_BILL_H__
#define __VALLEYOFSHADOWS_COLORADO_BILL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_colorado_bill : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(colorado_bill, equip_colorado_bill)

    struct equip_colorado_bill2 : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(colorado_bill2, equip_colorado_bill2)
}

#endif