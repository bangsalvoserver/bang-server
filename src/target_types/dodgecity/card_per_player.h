#ifndef __TARGET_TYPE_CARD_PER_PLAYER_H__
#define __TARGET_TYPE_CARD_PER_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_card_per_player : target_args::card {
        using value_type = card_list;

        targeting_card_per_player(target_args::card args) : target_args::card{args} {}

        const auto &get_args() const {
            return static_cast<const target_args::card &>(*this);
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