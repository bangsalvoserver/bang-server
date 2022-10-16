#ifndef __BASE_MUSTANG_H__
#define __BASE_MUSTANG_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_horse {
        game_string on_prompt(player *origin, card *target_card, player *target);
        void on_equip(card *target_card, player *target);
    };

    struct effect_mustang {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif