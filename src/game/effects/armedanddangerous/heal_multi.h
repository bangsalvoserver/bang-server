#ifndef __ARMEDANDDANGEROUS_DRAW_ATEND_H__
#define __ARMEDANDDANGEROUS_DRAW_ATEND_H__

#include "../card_effect.h"

namespace banggame {
    
    struct handler_heal_multi {
        game_string on_prompt(card *origin_card, player *origin, int amount);
        void on_play(card *origin_card, player *origin, int amount);
    };
}

#endif