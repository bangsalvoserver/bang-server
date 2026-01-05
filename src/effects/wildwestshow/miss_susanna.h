#ifndef __WILDWESTSHOW_MISS_SUSANNA_H___
#define __WILDWESTSHOW_MISS_SUSANNA_H___

#include "cards/card_effect.h"

namespace banggame {

    struct equip_miss_susanna : event_equip {
        int max_count;
        equip_miss_susanna(int max_count): max_count{max_count} {}

        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(miss_susanna, equip_miss_susanna)
}

#endif