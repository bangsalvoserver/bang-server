#ifndef __EQUIP_VTABLE_H__
#define __EQUIP_VTABLE_H__

#include "utils/tstring.h"

#include "cards/card_data.h"

namespace banggame {

    struct equip_vtable {
        std::string_view name;

        game_string (*on_prompt)(int effect_value, card *origin_card, player *origin, player *target);
        void (*on_enable)(int effect_value, card *target_card, player *target);
        void (*on_disable)(int effect_value, card *target_card, player *target);
        bool is_nodisable;
    };

    template<typename T>
    inline auto build_equip(int effect_value) {
        if constexpr (requires { T{effect_value}; }) {
            return T{effect_value};
        } else {
            return T{};
        }
    }
    
    template<utils::tstring Name, typename T>
    constexpr equip_vtable build_equip_vtable() {
        return {
            .name = std::string_view(Name),

            .on_prompt = [](int effect_value, card *origin_card, player *origin, player *target) -> game_string {
                auto value = build_equip<T>(effect_value);
                if constexpr (requires { value.on_check_target(origin_card, origin, target); }) {
                    if (player_is_bot(origin) && !value.on_check_target(origin_card, origin, target)) {
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
            static constexpr equip_vtable value = build_equip_vtable<#name, type>(); \
        };
    
    #define GET_EQUIP(name) (&equip_vtable_map<#name>::value)

    struct equip_none {};
    DEFINE_EQUIP(none, equip_none)

}

#endif