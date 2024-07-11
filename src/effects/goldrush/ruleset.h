#ifndef __GOLDRUSH_RULESET_H__
#define __GOLDRUSH_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_goldrush {
        void on_apply(game *game);
    };

    DEFINE_RULESET(goldrush, ruleset_goldrush)

}

#endif