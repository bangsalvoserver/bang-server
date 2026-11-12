#ifndef __GREATTRAINROBBERY_SWITCH_CARDS_H__
#define __GREATTRAINROBBERY_SWITCH_CARDS_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_switch_cards {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
        int evaluate(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(switch_cards, effect_switch_cards)
    
}

#endif
