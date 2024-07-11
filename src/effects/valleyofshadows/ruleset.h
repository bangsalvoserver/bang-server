#ifndef __VALLEYOFSHADOWS_RULESET_H__
#define __VALLEYOFSHADOWS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_valleyofshadows {
        void on_apply(game *game);
    };

    DEFINE_RULESET(valleyofshadows, ruleset_valleyofshadows)

}

#endif