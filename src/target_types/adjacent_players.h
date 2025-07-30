#ifndef __TARGET_TYPE_ADJACENT_PLAYERS_H__
#define __TARGET_TYPE_ADJACENT_PLAYERS_H__

#include "cards/card_effect.h"

namespace banggame {

    using player_pair = std::array<player_ptr, 2>;

    struct targeting_adjacent_players : targeting_base {
        using targeting_base::targeting_base;
        
        using value_type = player_pair;
        
        std::generator<player_pair> possible_targets(const effect_context &ctx);
        player_pair random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, player_pair target);
        prompt_string on_prompt(const effect_context &ctx, player_pair target);
        void add_context(effect_context &ctx, player_pair target);
        void on_play(const effect_context &ctx, player_pair target);
    };

    DEFINE_TARGETING(adjacent_players, targeting_adjacent_players)
}

#endif