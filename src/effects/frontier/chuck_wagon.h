#ifndef __FRONTIER_CHUCK_WAGON_H__
#define __FRONTIER_CHUCK_WAGON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_chuck_wagon : event_equip {
        int max_count;
        equip_chuck_wagon(int max_count): max_count{max_count} {}

        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(chuck_wagon, equip_chuck_wagon)
}

#endif