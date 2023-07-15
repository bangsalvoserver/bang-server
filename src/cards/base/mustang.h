#ifndef __BASE_MUSTANG_H__
#define __BASE_MUSTANG_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_horse {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_equip(card *target_card, player *target);
    };

    struct equip_mustang : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif