#ifndef __VTABLE_EQUIP_H__
#define __VTABLE_EQUIP_H__

#include "card_defs.h"

#include "utils/tstring.h"

#include "game/filters_simple.h"

namespace banggame {

    struct equip_vtable {
        std::string_view name;

        game_string (*on_prompt)(int effect_value, card *origin_card, player *origin, player *target);
        void (*on_enable)(int effect_value, card *target_card, player *target);
        void (*on_disable)(int effect_value, card *target_card, player *target);
        bool is_nodisable;
    };

    inline game_string equip_holder::on_prompt(card *origin_card, player *origin, player *target) const {
        return type->on_prompt(effect_value, origin_card, origin, target);
    }

    inline void equip_holder::on_enable(card *target_card, player *target) const {
        type->on_enable(effect_value, target_card, target);
    }

    inline void equip_holder::on_disable(card *target_card, player *target) const {
        type->on_disable(effect_value, target_card, target);
    }

    inline bool equip_holder::is_nodisable() const {
        return type->is_nodisable;
    }

    template<typename T>
    inline auto build_equip(int effect_value) {
        if constexpr (requires { T{effect_value}; }) {
            return T{effect_value};
        } else {
            return T{};
        }
    }
    
    template<typename T>
    constexpr equip_vtable build_equip_vtable(std::string_view name) {
        return {
            .name = name,

            .on_prompt = [](int effect_value, card *origin_card, player *origin, player *target) -> game_string {
                auto value = build_equip<T>(effect_value);
                if constexpr (requires { value.on_check_target(origin_card, origin, target); }) {
                    if (filters::is_player_bot(origin) && !value.on_check_target(origin_card, origin, target)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .on_enable = [](int effect_value, card *target_card, player *target) {
                auto value = build_equip<T>(effect_value);
                if constexpr (requires { value.on_enable(target_card, target); }) {
                    value.on_enable(target_card, target);
                }
            },

            .on_disable = [](int effect_value, card *target_card, player *target) {
                auto value = build_equip<T>(effect_value);
                if constexpr (requires { value.on_disable(target_card, target); }) {
                    value.on_disable(target_card, target);
                }
            },

            .is_nodisable = requires { typename T::nodisable; }
        };
    }
    
    template<utils::tstring Name> struct equip_vtable_map;

    #define DEFINE_EQUIP(name, type) \
        template<> struct equip_vtable_map<#name> { \
            static constexpr equip_vtable value = build_equip_vtable<type>(#name); \
        };
    
    #define GET_EQUIP(name) (&equip_vtable_map<#name>::value)

}

#endif