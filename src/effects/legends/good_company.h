#ifndef __LEGENDS_GOOD_COMPANY_H__
#define __LEGENDS_GOOD_COMPANY_H__

#include "cards/card_effect.h"

#include "perform_feat.h"

#include "effects/base/steal_destroy.h"

namespace banggame {
    
    struct equip_good_company : feat_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(good_company, equip_good_company)

    struct effect_good_company : effect_discard {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target);
    };

    DEFINE_EFFECT(good_company, effect_good_company)
}

#endif