#ifndef __TARGET_TYPE_CARD_PER_PLAYER_H__
#define __TARGET_TYPE_CARD_PER_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_card_per_player {
        using value_type = card_list;

        player_filter_bitset player_filter;
        card_filter_bitset card_filter;

        targeting_card_per_player(targeting_args<void, target_filter::card> args)
            : player_filter{args.player_filter}
            , card_filter{args.card_filter} {}

        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                card_filter_bitset card_filter;
            };
            return args{ player_filter, card_filter };
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return true;
        }
        
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(card_per_player, targeting_card_per_player)
}

#endif