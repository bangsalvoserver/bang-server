#ifndef __LEGENDS_OLD_WEST_GANG_H__
#define __LEGENDS_OLD_WEST_GANG_H__

#include "cards/card_effect.h"

#include "perform_feat.h"

namespace banggame {

    struct equip_old_west_gang : feat_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(old_west_gang, equip_old_west_gang)
}

#endif