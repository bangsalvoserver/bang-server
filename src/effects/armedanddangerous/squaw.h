#ifndef __ARMEDANDDANGEROUS_SQUAW_H__
#define __ARMEDANDDANGEROUS_SQUAW_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct handler_squaw {
        bool on_check_target(card *origin_card, player *origin, card *discarded_card, bool paid_cubes) {
            return bot_suggestion::target_enemy_card{}.on_check_target(origin_card, origin, discarded_card);
        }
        game_string get_error(card *origin_card, player *origin, card *discarded_card, bool paid_cubes);
        game_string on_prompt(card *origin_card, player *origin, card *discarded_card, bool paid_cubes);
        void on_play(card *origin_card, player *origin, card *discarded_card, bool paid_cubes);
    };

    DEFINE_MTH(squaw, handler_squaw)
}

#endif