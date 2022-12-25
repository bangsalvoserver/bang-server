#ifndef __ARMEDANDDANGEROUS_SQUAW_H__
#define __ARMEDANDDANGEROUS_SQUAW_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_squaw {
        game_string on_prompt(card *origin_card, player *origin, card *discarded_card, opt_tagged_value<target_type::none> paid_cubes);
        void on_play(card *origin_card, player *origin, card *discarded_card, opt_tagged_value<target_type::none> paid_cubes);
    };
}

#endif