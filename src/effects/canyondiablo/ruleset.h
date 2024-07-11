#ifndef __CANYONDIABLO_RULESET_H__
#define __CANYONDIABLO_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_canyondiablo {
        void on_apply(game *game);
    };

    DEFINE_RULESET(canyondiablo, ruleset_canyondiablo)

}

#endif