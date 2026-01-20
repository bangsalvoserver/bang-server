#ifndef __TARGET_TYPE_ADJACENT_PLAYERS_H__
#define __TARGET_TYPE_ADJACENT_PLAYERS_H__

#include "target_types/base/player.h"

namespace banggame {

    struct targeting_adjacent_players {
        using value_type = player_list;

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
        
        std::generator<player_list> possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const player_list &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const player_list &target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const player_list &target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const player_list &target);
    };

    DEFINE_TARGETING(adjacent_players, targeting_adjacent_players)
}

#endif