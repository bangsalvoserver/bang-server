#ifndef __LEGENDS_FIFTY_GUNS_H__
#define __LEGENDS_FIFTY_GUNS_H__

#include "cards/card_effect.h"

#include "perform_feat.h"

namespace banggame {

    struct equip_fifty_guns : feat_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(fifty_guns, equip_fifty_guns)
}

#endif