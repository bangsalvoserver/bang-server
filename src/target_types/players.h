#ifndef __TARGET_TYPE_PLAYERS_H__
#define __TARGET_TYPE_PLAYERS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_players : targeting_base {
        using targeting_base::targeting_base;

        struct value_type{};
        
        std::generator<value_type> possible_targets(const effect_context &ctx) {
            if (!get_error(ctx, {})) {
                co_yield {};
            }
        }

        value_type random_target(const effect_context &ctx) {
            return {};
        }

        game_string get_error(const effect_context &ctx, value_type);
        prompt_string on_prompt(const effect_context &ctx, value_type);
        void add_context(effect_context &ctx, value_type);
        void on_play(const effect_context &ctx, value_type);
    };

    DEFINE_TARGETING(players, targeting_players)
}

#endif