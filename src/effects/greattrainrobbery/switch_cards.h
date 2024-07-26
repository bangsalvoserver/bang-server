#ifndef __GREATTRAINROBBERY_SWITCH_CARDS_H__
#define __GREATTRAINROBBERY_SWITCH_CARDS_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {
    
    struct handler_switch_cards {
        bool on_check_target(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
            return bot_suggestion::target_enemy_card{}.on_check_target(origin_card, origin, target_card);
        }
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    };

    DEFINE_MTH(switch_cards, handler_switch_cards)
}

#endif