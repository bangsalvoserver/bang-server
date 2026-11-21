#ifndef __ARMEDANDDANGEROUS_SQUAW_H__
#define __ARMEDANDDANGEROUS_SQUAW_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_squaw_target {
        int evaluate(card_ptr origin_card, player_ptr origin, card_ptr target_card) const;
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_context &ctx) const;
    };

    DEFINE_EFFECT(squaw_target, effect_squaw_target)

    struct effect_squaw {
        game_string get_error(card_ptr origin_card, player_ptr origin, int ncubes, const effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, int ncubes, effect_flags flags, const effect_context &ctx) const;
        int evaluate(card_ptr origin_card, player_ptr origin, int ncubes) const;
        void on_play(card_ptr origin_card, player_ptr origin, int ncubes, effect_flags flags, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(squaw, effect_squaw)
}

#endif
