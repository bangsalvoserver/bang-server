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

}

#endif