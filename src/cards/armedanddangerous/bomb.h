#ifndef __ARMEDANDDANGEROUS_BOMB_H__
#define __ARMEDANDDANGEROUS_BOMB_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_move_bomb {
        bool can_respond(card *origin_card, player *origin);
    };

    struct handler_move_bomb {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        game_string verify(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };
    
    struct equip_bomb {
        void on_equip(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };
}

#endif