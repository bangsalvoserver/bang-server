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
    inline void do_apply_ruleset(game *game) {
        if constexpr (requires { ruleset_vtable_map<E>::value; }) {
            if (game->m_options.expansions.check(E)) {
                ruleset_vtable_map<E>::value.on_apply(game);
            }
        }
    }

    inline void apply_rulesets(game *game) {
        [game]<expansion_type ... Es>(enums::enum_sequence<Es ...>) {
            (do_apply_ruleset<Es>(game), ...);
        }(enums::make_enum_sequence<expansion_type>());
    }
}

#endif