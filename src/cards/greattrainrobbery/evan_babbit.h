#ifndef __GREATTRAINROBBERY_EVAN_BABBIT_H__
#define __GREATTRAINROBBERY_EVAN_BABBIT_H__

#include "cards/card_effect.h"

#include "game/bot_suggestion.h"

namespace banggame {

    struct handler_evan_babbit {
        bool on_check_target(card *origin_card, player *origin, card *target_card, player *target_player) {
            return bot_suggestion::target_enemy{}.on_check_target(origin_card, origin, target_player);
        }
        game_string get_error(card *origin_card, player *origin, card *target_card, player *target_player);
        void on_play(card *origin_card, player *origin, card *target_card, player *target_player);
    };
}

#endif