#ifndef __VTABLE_EFFECT_H__
#define __VTABLE_EFFECT_H__

#include "card_defs.h"

#include "utils/fixed_string.h"

namespace banggame {

    struct effect_vtable {
        std::string_view name;

        bool (*can_play)(const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        
        game_string (*get_error)(const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        prompt_string (*on_prompt)(const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void (*add_context)(const void *effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx);
        void (*on_play)(const void *effect_value, card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx);

        game_string (*get_error_player)(const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx);
        prompt_string (*on_prompt_player)(const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx);
        void (*add_context_player)(const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx);
        void (*on_play_player)(const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx);
        
        game_string (*get_error_card)(const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx);
        prompt_string (*on_prompt_card)(const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx);
        void (*add_context_card)(const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx);
        void (*on_play_card)(const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx);
    };
    
    inline bool effect_holder::can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        return type->can_play(effect_value, origin_card, origin, ctx);
    }
    
    inline game_string effect_holder::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        return type->get_error(effect_value, origin_card, origin, ctx);
    }

    inline game_string effect_holder::get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        return type->get_error_player(effect_value, origin_card, origin, target, ctx);
    }

    inline game_string effect_holder::get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const {
        return type->get_error_card(effect_value, origin_card, origin, target, ctx);
    }

    inline prompt_string effect_holder::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        return type->on_prompt(effect_value, origin_card, origin, ctx);
    }

    inline prompt_string effect_holder::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        return type->on_prompt_player(effect_value, origin_card, origin, target, ctx);
    }

    inline prompt_string effect_holder::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const {
        return type->on_prompt_card(effect_value, origin_card, origin, target, ctx);
    }

    inline void effect_holder::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const {
        type->add_context(effect_value, origin_card, origin, ctx);
    }

    inline void effect_holder::add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) const {
        type->add_context_player(effect_value, origin_card, origin, target, ctx);
    }

    inline void effect_holder::add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) const {
        type->add_context_card(effect_value, origin_card, origin, target, ctx);
    }

    inline void effect_holder::on_play(card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) const {
        type->on_play(effect_value, origin_card, origin, flags, ctx);
    }

    inline void effect_holder::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) const {
        type->on_play_player(effect_value, origin_card, origin, target, flags, ctx);
    }

    inline void effect_holder::on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) const {
        type->on_play_card(effect_value, origin_card, origin, target, flags, ctx);
    }
    
    template<utils::fixed_string Name> struct effect_vtable_map;

    #define BUILD_EFFECT_VTABLE(name, type)
    #define DEFINE_EFFECT(NAME, TYPE) \
        template<> struct effect_vtable_map<#NAME> { using type = TYPE; static const effect_vtable value; }; \
        BUILD_EFFECT_VTABLE(NAME, TYPE)
    
    #define GET_EFFECT(name) (&effect_vtable_map<#name>::value)
    #define BUILD_EFFECT_VALUE(name, ...) (effect_vtable_map<#name>::type{__VA_ARGS__})
    
    struct equip_vtable {
        std::string_view name;

        prompt_string (*on_prompt)(const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target);
        void (*on_enable)(const void *effect_value, card_ptr target_card, player_ptr target);
        void (*on_disable)(const void *effect_value, card_ptr target_card, player_ptr target);
        bool is_nodisable;
    };

    inline prompt_string equip_holder::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return type->on_prompt(effect_value, origin_card, origin, target);
    }

    inline void equip_holder::on_enable(card_ptr target_card, player_ptr target) const {
        type->on_enable(effect_value, target_card, target);
    }

    inline void equip_holder::on_disable(card_ptr target_card, player_ptr target) const {
        type->on_disable(effect_value, target_card, target);
    }

    inline bool equip_holder::is_nodisable() const {
        return type->is_nodisable;
    }
    
    template<utils::fixed_string Name> struct equip_vtable_map;

    #define BUILD_EQUIP_VTABLE(name, type)
    #define DEFINE_EQUIP(NAME, TYPE) \
        template<> struct equip_vtable_map<#NAME> { using type = TYPE; static const equip_vtable value; }; \
        BUILD_EQUIP_VTABLE(NAME, TYPE)
    
    #define GET_EQUIP(name) (&equip_vtable_map<#name>::value)
    #define BUILD_EQUIP_VALUE(name, ...) (equip_vtable_map<#name>::type{__VA_ARGS__})
    
    struct modifier_vtable {
        std::string_view name;

        void (*add_context)(const void *effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx);
        game_string (*get_error)(const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx);
    };

    inline void modifier_holder::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const {
        type->add_context(effect_value, origin_card, origin, ctx);
    }

    inline game_string modifier_holder::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const {
        return type->get_error(effect_value, origin_card, origin, target_card, ctx);
    }

    template<utils::fixed_string Name> struct modifier_vtable_map;

    #define BUILD_MODIFIER_VTABLE(name, type)
    #define DEFINE_MODIFIER(NAME, TYPE) \
        template<> struct modifier_vtable_map<#NAME> { using type = TYPE; static const modifier_vtable value; }; \
        BUILD_MODIFIER_VTABLE(NAME, TYPE)
    
    #define GET_MODIFIER(name) (&modifier_vtable_map<#name>::value)
    #define BUILD_MODIFIER_VALUE(name, ...) (modifier_vtable_map<#name>::type{__VA_ARGS__})

    struct mth_vtable {
        std::string_view name;
        
        game_string (*get_error)(const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx);
        prompt_string (*on_prompt)(const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx);
        void (*on_play)(const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx);
    };

    inline game_string mth_holder::get_error(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const {
        return type->get_error(effect_value, origin_card, origin, targets, args, ctx);
    }

    inline prompt_string mth_holder::on_prompt(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const {
        return type->on_prompt(effect_value, origin_card, origin, targets, args, ctx);
    }

    inline void mth_holder::on_play(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const {
        type->on_play(effect_value, origin_card, origin, targets, args, ctx);
    }

    template<utils::fixed_string Name> struct mth_vtable_map;

    #define BUILD_MTH_VTABLE(name, type)
    #define DEFINE_MTH(NAME, TYPE) \
        template<> struct mth_vtable_map<#NAME> { using type = TYPE; static const mth_vtable value; }; \
        BUILD_MTH_VTABLE(NAME, TYPE)
    
    #define GET_MTH(name) (&mth_vtable_map<#name>::value)
    #define BUILD_MTH_VALUE(name, ...) (mth_vtable_map<#name>::type{__VA_ARGS__})
    
    struct ruleset_vtable {
        std::string_view name;

        void (*on_apply)(game_ptr game);
        bool (*is_valid_with)(const expansion_set &set);
    };

    template<utils::fixed_string Name>
    struct ruleset_vtable_map;

    #define BUILD_RULESET_VTABLE(name, type)
    #define DEFINE_RULESET(name, type) \
        template<> struct ruleset_vtable_map<#name> { static const ruleset_vtable value; }; \
        BUILD_RULESET_VTABLE(name, type)
    
    #define GET_RULESET(name) (&ruleset_vtable_map<#name>::value)
}

#endif