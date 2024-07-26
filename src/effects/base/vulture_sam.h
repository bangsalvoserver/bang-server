#ifndef __BASE_VULTURE_SAM_H__
#define __BASE_VULTURE_SAM_H__

#include "cards/card_effect.h"

namespace banggame {

    enum class card_taker_type {
        dead_players,
        discards,
        draw_checks
    };

    namespace event_type {
        struct check_card_taker {
            player_ptr target;
            card_taker_type type;
            nullable_ref<card_ptr> value;
        };
    }
    
    struct equip_vulture_sam : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(vulture_sam, equip_vulture_sam)
}

#endif