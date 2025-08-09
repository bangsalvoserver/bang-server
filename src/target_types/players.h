#ifndef __TARGET_TYPE_PLAYERS_H__
#define __TARGET_TYPE_PLAYERS_H__

#include "player.h"

namespace banggame {

    struct targeting_players {
        using value_type = std::nullptr_t;

        targeting_player target_player;

        targeting_players(targeting_args<void, target_filter::player> args)
            : target_player{{ .player_filter = args.player_filter }} {}
        
        auto get_args() const {
            return target_player.get_args();
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return !get_error(origin_card, origin, effect, ctx, {});
        }

        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return {};
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type);
    };

    DEFINE_TARGETING(players, targeting_players)
}

#endif