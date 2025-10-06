#ifndef __TARGET_TYPE_ADJACENT_PLAYERS_H__
#define __TARGET_TYPE_ADJACENT_PLAYERS_H__

#include "target_types/base/player.h"

namespace banggame {

    using player_pair = std::array<player_ptr, 2>;

    struct targeting_adjacent_players {
        using value_type = player_pair;

        targeting_player target_player;
        int max_distance;

        targeting_adjacent_players(target_args::player args, int max_distance)
            : target_player{args}
            , max_distance{max_distance} {}
        
        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                int max_distance;
            };
            return args{ target_player.player_filter, max_distance };
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            auto all_players = origin->m_game->range_all_players(origin);
            return rv::cartesian_product(all_players, all_players)
                | rv::transform([](const auto &pair) {
                    auto [target1, target2] = pair;
                    return player_pair{target1, target2};
                })
                | rv::filter([=, &ctx, this](player_pair targets) {
                    return !get_error(origin_card, origin, effect, ctx, targets);
                });
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_pair target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_pair target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_pair target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_pair target);
    };

    DEFINE_TARGETING(adjacent_players, targeting_adjacent_players)
}

#endif