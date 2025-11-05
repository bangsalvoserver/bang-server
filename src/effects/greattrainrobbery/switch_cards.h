#ifndef __GREATTRAINROBBERY_SWITCH_CARDS_H__
#define __GREATTRAINROBBERY_SWITCH_CARDS_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_switch_cards {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card);
    };

    DEFINE_EFFECT(switch_cards, effect_switch_cards)
}

#endif