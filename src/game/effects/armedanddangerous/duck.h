#ifndef __ARMEDANDDANGEROUS_DUCK_H__
#define __ARMEDANDDANGEROUS_DUCK_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_duck {
        game_string on_prompt(card *origin_card, player *origin, opt_tagged_value<target_type::none> paid_cubes);
        void on_play(card *origin_card, player *origin, opt_tagged_value<target_type::none> paid_cubes);
    };
}

#endif