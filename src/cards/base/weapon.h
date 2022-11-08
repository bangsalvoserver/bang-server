#ifndef __BASE_WEAPON_H__
#define __BASE_WEAPON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_weapon_base {
        int range;
        effect_weapon_base(int value) : range(value) {}

        void on_equip(card *target_card, player *target);
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct effect_weapon : effect_weapon_base {
        effect_weapon(int value) : effect_weapon_base(value) {}
        game_string on_prompt(player *origin, card *target_card, player *target);
    };
}

#endif