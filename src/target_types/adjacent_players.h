#ifndef __TARGET_TYPE_ADJACENT_PLAYERS_H__
#define __TARGET_TYPE_ADJACENT_PLAYERS_H__

#include "cards/card_effect.h"

#include "game/possible_to_play.h"

namespace banggame {

    using player_pair = std::array<player_ptr, 2>;

    struct targeting_adjacent_players : targeting_base {
        using targeting_base::targeting_base;
        
        using value_type = player_pair;
        
        auto possible_targets(const effect_context &ctx) {
            return rv::cartesian_product(origin->m_game->m_players, origin->m_game->m_players)
                | rv::transform([](const auto &pair) {
                    auto [target1, target2] = pair;
                    return player_pair{target1, target2};
                })
                | rv::filter([this, &ctx](player_pair targets) {
                    return !get_error(ctx, targets);
                });
        }

        player_pair random_target(const effect_context &ctx) {
            return random_element(possible_targets(ctx), origin->m_game->bot_rng);
        }

        game_string get_error(const effect_context &ctx, player_pair target);
        prompt_string on_prompt(const effect_context &ctx, player_pair target);
        void add_context(effect_context &ctx, player_pair target);
        void on_play(const effect_context &ctx, player_pair target);
    };

    DEFINE_TARGETING(adjacent_players, targeting_adjacent_players)
}

#endif