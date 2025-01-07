#ifndef __LEGENDS_TRAIN_TO_YOOMA_H__
#define __LEGENDS_TRAIN_TO_YOOMA_H__

#include "cards/card_effect.h"

#include "perform_feat.h"

namespace banggame {

    struct equip_train_to_yooma : feat_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(train_to_yooma, equip_train_to_yooma)
}

#endif