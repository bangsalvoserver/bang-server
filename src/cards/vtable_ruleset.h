#ifndef __VTABLE_RULESET_H__
#define __VTABLE_RULESET_H__

#include "card_defs.h"

namespace banggame {

    struct ruleset_vtable {
        void (*on_apply)(game *game);
    };

    template<typename T>
    constexpr ruleset_vtable build_ruleset_vtable() {
        return {
            .on_apply = [](game *game) {
                T{}.on_apply(game);
            }
        };
    }

    template<expansion_type E> struct ruleset_vtable_map;

    #define DEFINE_RULESET(expansion, type) \
        template<> struct ruleset_vtable_map<expansion_type::expansion> { \
            static constexpr ruleset_vtable value = build_ruleset_vtable<type>(); \
        };
    
    template<expansion_type E>
    constexpr const ruleset_vtable *get_ruleset_vtable() {
        if constexpr ( requires { ruleset_vtable_map<E>::value; }) {
            return &ruleset_vtable_map<E>::value;
        } else {
            return nullptr;
        }
    }

    inline const ruleset_vtable *get_ruleset_vtable(expansion_type expansion) {
        static constexpr const auto &values = enums::enum_values<expansion_type>();
        static constexpr auto lookup = []<size_t ... Is>(std::index_sequence<Is...>) {
            return std::array { get_ruleset_vtable<values[Is]>() ... };
        }(std::make_index_sequence<values.size()>());
        
        return lookup[enums::indexof(expansion)];
    }

}

#endif