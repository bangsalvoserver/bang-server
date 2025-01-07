#ifndef __LEGENDS_IN_GOOD_COMPANY_H__
#define __LEGENDS_IN_GOOD_COMPANY_H__

#include "cards/card_effect.h"

#include "effects/base/steal_destroy.h"

namespace banggame {
    
    struct equip_in_good_company : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(in_good_company, equip_in_good_company)

    struct effect_in_good_company : effect_discard {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(in_good_company, effect_in_good_company)
}

#endif