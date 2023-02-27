#ifndef __HOLDERS_H__
#define __HOLDERS_H__

#include "cards/card_effect.h"

namespace banggame {
    
    DEFINE_STRUCT(effect_holder,
        (target_type, target)
        (target_player_filter, player_filter)
        (target_card_filter, card_filter)
        (int8_t, effect_value)
        (int8_t, target_value)
        (effect_type, type),

        game_string get_error(card *origin_card, player *origin, const effect_context &ctx = {}) const;
        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx = {}) const;
        void on_play(card *origin_card, player *origin, effect_flags flags = {}, const effect_context &ctx = {}) const;

        game_string get_error(card *origin_card, player *origin, player *target, const effect_context &ctx = {}) const;
        game_string on_prompt(card *origin_card, player *origin, player *target, const effect_context &ctx = {}) const;
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {}, const effect_context &ctx = {}) const;
        
        game_string get_error(card *origin_card, player *origin, card *target, const effect_context &ctx = {}) const;
        game_string on_prompt(card *origin_card, player *origin, card *target, const effect_context &ctx = {}) const;
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {}, const effect_context &ctx = {}) const;
    )
    
    DEFINE_STRUCT(equip_holder,
        (short, effect_value)
        (equip_type, type),

        game_string on_prompt(card *origin_card, player *origin, player *target) const;
        void on_equip(card *target_card, player *target) const;
        void on_enable(card *target_card, player *target) const;
        void on_disable(card *target_card, player *target) const;
        void on_unequip(card *target_card, player *target) const;
    )

    DEFINE_STRUCT(modifier_holder,
        (modifier_type, type),
        
        void add_context(card *origin_card, player *origin, effect_context &ctx) const;
        void add_context(card *origin_card, player *origin, card *target, effect_context &ctx) const;
        void add_context(card *origin_card, player *origin, player *target, effect_context &ctx) const;

        game_string get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx) const;
        game_string on_prompt(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) const;
    )

    DEFINE_STRUCT(mth_holder,
        (mth_type, type),
        
        game_string get_error(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx = {}) const;
        game_string on_prompt(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx = {}) const;
        void on_play(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx = {}) const;
    )

    DEFINE_STRUCT(tag_holder,
        (short, tag_value)
        (tag_type, type)
    )
}

#endif