#ifndef __GREATTRAINROBBERY_SWITCH_CARDS_H__
#define __GREATTRAINROBBERY_SWITCH_CARDS_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct handler_switch_cards {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    };

    DEFINE_MTH(switch_cards, handler_switch_cards)

    struct effect_switch_self {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return handler_switch_cards{}.get_error(origin_card, origin, origin_card, target_card);
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return handler_switch_cards{}.on_prompt(origin_card, origin, origin_card, target_card);
        }

        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return handler_switch_cards{}.on_play(origin_card, origin, origin_card, target_card);
        }
    };

    DEFINE_EFFECT(switch_self, effect_switch_self)
}

#endif