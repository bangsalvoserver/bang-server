#ifndef __FRONTIER_RULESET_H__
#define __FRONTIER_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_frontier {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(frontier, ruleset_frontier)

}

#endif