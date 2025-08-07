#ifndef __TARGET_TYPE_NONE_H__
#define __TARGET_TYPE_NONE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_none : targeting_base {
        using targeting_base::targeting_base;
        
        using value_type = std::nullptr_t;
        
        bool is_possible(const effect_context &ctx) {
            return !get_error(ctx, {});
        }

        value_type random_target(const effect_context &ctx) {
            return {};
        }
        
        game_string get_error(const effect_context &ctx, value_type) {
            return effect.get_error(origin_card, origin, ctx);
        }

        prompt_string on_prompt(const effect_context &ctx, value_type) {
            return effect.on_prompt(origin_card, origin, ctx);
        }

        void add_context(effect_context &ctx, value_type) {
            effect.add_context(origin_card, origin, ctx);
        }

        void on_play(const effect_context &ctx, value_type) {
            effect.on_play(origin_card, origin, {}, ctx);
        }
    };

    DEFINE_TARGETING(none, targeting_none)
}

#endif