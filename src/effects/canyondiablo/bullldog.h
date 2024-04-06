#ifndef __CANYONDIABLO_BULLDOG_H__
#define __CANYONDIABLO_BULLDOG_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_bulldog {
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, card *discarded_card);
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, card *discarded_card);
        void on_play(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, card *discarded_card);
    };

    DEFINE_MTH(bulldog, handler_bulldog)

}

#endif