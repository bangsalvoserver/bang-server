#ifndef __CANYONDIABLO_BULLDOG_H__
#define __CANYONDIABLO_BULLDOG_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_bulldog {
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, card_ptr discarded_card);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, card_ptr discarded_card);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, card_ptr discarded_card);
    };

    DEFINE_MTH(bulldog, handler_bulldog)

}

#endif