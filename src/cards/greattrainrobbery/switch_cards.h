#ifndef __GREATTRAINROBBERY_SWITCH_CARDS_H__
#define __GREATTRAINROBBERY_SWITCH_CARDS_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {
    
    struct handler_switch_cards {
        bool on_check_target(card *origin_card, player *origin, card *chosen_card, card *target_card) {
            return bot_suggestion::target_enemy_card{}.on_check_target(origin_card, origin, target_card);
        }
        game_string get_error(card *origin_card, player *origin, card *chosen_card, card *target_card);
        void on_play(card *origin_card, player *origin, card *chosen_card, card *target_card);
    };

    DEFINE_MTH(switch_cards, handler_switch_cards)
}

#endif