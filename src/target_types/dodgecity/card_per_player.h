#ifndef __TARGET_TYPE_CARD_PER_PLAYER_H__
#define __TARGET_TYPE_CARD_PER_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    using targeting_card_per_player_args = targeting_args<void, target_filter::card>;

    struct targeting_card_per_player : targeting_card_per_player_args {
        using value_type = card_list;

        targeting_card_per_player(targeting_card_per_player_args args) : targeting_card_per_player_args{args} {}

        const auto &get_args() const {
            return static_cast<const targeting_card_per_player_args &>(*this);
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