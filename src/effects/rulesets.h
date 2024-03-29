#ifndef __EFFECTS_H__
#define __EFFECTS_H__

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
    inline void do_apply_ruleset(game *game, enums::enum_tag_t<E>) {
        if constexpr (enums::value_with_type<E>) {
            if (bool(game->m_options.expansions & E)) {
                using type = enums::enum_type_t<E>;
                type{}.on_apply(game);
            }
        }
    }

    inline void apply_rulesets(game *game) {
        [&]<expansion_type ... Es>(enums::enum_sequence<Es ...>) {
            (do_apply_ruleset(game, enums::enum_tag<Es>), ...);
        }(enums::make_enum_sequence<expansion_type>());
    }
}

#endif