#ifndef __VTABLE_RULESET_H__
#define __VTABLE_RULESET_H__

#include "card_defs.h"

#include "utils/fixed_string.h"

namespace banggame {

    struct ruleset_vtable {
        std::string_view name;

        void (*on_apply)(game *game);
    };

    template<typename T>
    constexpr ruleset_vtable build_ruleset_vtable(std::string_view name) {
        return {
            .name = name,

            .on_apply = [](game *game) {
                T{}.on_apply(game);
            }
        };
    }

    template<utils::fixed_string Name>
    struct ruleset_vtable_map;

    #define DEFINE_RULESET(name, type) \
        template<> struct ruleset_vtable_map<#name> { \
            static constexpr ruleset_vtable value = build_ruleset_vtable<type>(#name); \
        };
    
    #define GET_RULESET(name) (&ruleset_vtable_map<#name>::value)

}

#endif