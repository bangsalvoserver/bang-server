#ifndef __RULESETS_H__
#define __RULESETS_H__

#include "armedanddangerous/ruleset.h"
#include "canyondiablo/ruleset.h"
#include "dodgecity/ruleset.h"
#include "fistfulofcards/ruleset.h"
#include "goldrush/ruleset.h"
#include "greattrainrobbery/ruleset.h"
#include "highnoon/ruleset.h"
#include "valleyofshadows/ruleset.h"
    
namespace banggame {

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