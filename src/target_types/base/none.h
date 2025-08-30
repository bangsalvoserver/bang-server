#ifndef __TARGET_TYPE_NONE_H__
#define __TARGET_TYPE_NONE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct targeting_none {
        using value_type = std::nullptr_t;

        targeting_none(targeting_args<void, target_filter::none> = {}) {}
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return !get_error(origin_card, origin, effect, ctx);
        }

        value_type random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return {};
        }
        
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            return effect.get_error(origin_card, origin, ctx);
        }

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            return effect.on_prompt(origin_card, origin, ctx);
        }

        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type = {}) {
            effect.add_context(origin_card, origin, ctx);
        }

        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type = {}) {
            effect.on_play(origin_card, origin, {}, ctx);
        }
    };

    DEFINE_TARGETING(none, targeting_none)
}

#endif