#ifndef __BASE_WEAPON_H__
#define __BASE_WEAPON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_weapon_base {
        int range;
        equip_weapon_base(int value) : range(value) {}

        void on_equip(card *target_card, player *target);
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct equip_weapon : equip_weapon_base {
        equip_weapon(int value) : equip_weapon_base(value) {}
        game_string on_prompt(player *origin, card *target_card, player *target);
    };
}

#endif