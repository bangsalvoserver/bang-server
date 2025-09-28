#ifndef __TARGET_TYPE_PLAYERS_H__
#define __TARGET_TYPE_PLAYERS_H__

#include "cards/card_effect.h"

#include "game/game_table.h"

namespace banggame {

    struct targeting_players : target_args::player {
        using value_type = std::nullptr_t;

        targeting_players(target_args::player args) : target_args::player{args} {}
        
        const auto &get_args() const {
            return static_cast<const target_args::player &>(*this);
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return !get_error(origin_card, origin, effect, ctx);
        }

        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return {};
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {});
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {});
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type = {});
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {});
    };

    DEFINE_TARGETING(players, targeting_players)
}

#endif