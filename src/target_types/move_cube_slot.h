#ifndef __TARGET_TYPE_MOVE_CUBE_SLOT_H__
#define __TARGET_TYPE_MOVE_CUBE_SLOT_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_move_cube_slot : targeting_base {
        using targeting_base::targeting_base;
        
        using value_type = card_list;
        
        bool is_possible(const effect_context &ctx);
        card_list random_target(const effect_context &ctx);
        game_string get_error(const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(const effect_context &ctx, const card_list &target) { return {}; }
        void add_context(effect_context &ctx, const card_list &target) {}
        void on_play(const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(move_cube_slot, targeting_move_cube_slot)
}

#endif