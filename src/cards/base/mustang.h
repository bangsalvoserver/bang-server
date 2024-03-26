#ifndef __BASE_MUSTANG_H__
#define __BASE_MUSTANG_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_horse {
        struct nodisable {};
        
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(horse, equip_horse)

    struct equip_mustang : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(mustang, equip_mustang)
}

#endif