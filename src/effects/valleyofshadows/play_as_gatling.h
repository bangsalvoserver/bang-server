#ifndef __VALLEYOFSHADOWS_PLAY_AS_GATLING_H__
#define __VALLEYOFSHADOWS_PLAY_AS_GATLING_H__

#include "cards/card_effect.h"

namespace banggame {

    struct handler_play_as_gatling {
        game_string get_error(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card);
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card);
        void on_play(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card);
    };

    DEFINE_MTH(play_as_gatling, handler_play_as_gatling)

}

#endif