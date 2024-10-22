#ifndef __VALLEYOFSHADOWS_RULESET_H__
#define __VALLEYOFSHADOWS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_valleyofshadows {
        void on_apply(game *game);
    };

    DEFINE_RULESET(valleyofshadows, ruleset_valleyofshadows)

    struct ruleset_udolistinu {
        void on_apply(game *game);
        bool is_valid_with(const expansion_set &set);
    };

    DEFINE_RULESET(udolistinu, ruleset_udolistinu)

}

#endif