#ifndef __CANYONDIABLO_LAURA_BILLION__
#define __CANYONDIABLO_LAURA_BILLION__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_laura_billion : event_equip {
        void on_enable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(laura_billion, equip_laura_billion)

    struct effect_laura_billion {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(laura_billion, effect_laura_billion)
}

#endif