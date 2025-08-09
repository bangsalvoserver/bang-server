#ifndef __TARGET_TYPE_EXTRA_CARD_H__
#define __TARGET_TYPE_EXTRA_CARD_H__

#include "card.h"

namespace banggame {

    struct targeting_extra_card : targeting_card {
        using targeting_card::targeting_card;

        using value_type = nullable_card;
        
        std::generator<card_ptr> possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(extra_card, targeting_extra_card)
}

#endif