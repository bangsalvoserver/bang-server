#ifndef __WILDWESTSHOW_BONE_ORCHARD_H___
#define __WILDWESTSHOW_BONE_ORCHARD_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bone_orchard : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(bone_orchard, equip_bone_orchard)
}

#endif