#ifndef __TARGET_TYPE_CARD_PER_PLAYER_H__
#define __TARGET_TYPE_CARD_PER_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_card_per_player : targeting_base {
        using targeting_base::targeting_base;

        using value_type = card_list;
        
        std::generator<card_list> possible_targets(const effect_context &ctx) {
            co_yield {};
        }
        
        card_list random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(const effect_context &ctx, const card_list &target);
        void add_context(effect_context &ctx, const card_list &target);
        void on_play(const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(card_per_player, targeting_card_per_player)
}

#endif