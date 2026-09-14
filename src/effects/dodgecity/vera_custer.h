#ifndef __DODGECITY_VERA_CUSTER_H__
#define __DODGECITY_VERA_CUSTER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vera_custer {
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(vera_custer, equip_vera_custer)

    namespace event_type {
        struct check_card_copied {
            using result_type = bool;
            const_card_ptr target_card;
        };
    }
}

#endif