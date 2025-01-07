#ifndef __LEGENDS_MAN_WITH_NO_NAME_H__
#define __LEGENDS_MAN_WITH_NO_NAME_H__

#include "cards/card_effect.h"

#include "perform_feat.h"

namespace banggame {

    struct equip_man_with_no_name : feat_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(man_with_no_name, equip_man_with_no_name)
}

#endif