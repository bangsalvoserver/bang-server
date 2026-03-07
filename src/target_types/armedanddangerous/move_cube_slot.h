#ifndef __TARGET_TYPE_MOVE_CUBE_SLOT_H__
#define __TARGET_TYPE_MOVE_CUBE_SLOT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_move_cube_slot_args {
        [[=json::rename("max_cubes")]]
        int ncubes;
    };

    struct targeting_move_cube_slot : targeting_move_cube_slot_args {
        using value_type = card_list;

        targeting_move_cube_slot(target_args::empty, int ncubes)
            : targeting_move_cube_slot_args{ncubes} {}
        
        const auto &get_args() const {
            return static_cast<const targeting_move_cube_slot_args &>(*this);
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target) { return {}; }
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target) {}
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(move_cube_slot, targeting_move_cube_slot)
}

#endif