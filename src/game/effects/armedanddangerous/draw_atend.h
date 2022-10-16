#ifndef __ARMEDANDDANGEROUS_DRAW_ATEND_H__
#define __ARMEDANDDANGEROUS_DRAW_ATEND_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_draw_atend {
        void on_play(card *origin_card, player *origin, int amount);
    };
}

#endif