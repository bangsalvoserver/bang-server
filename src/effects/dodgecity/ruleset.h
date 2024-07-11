#ifndef __DODGECITY_RULESET_H__
#define __DODGECITY_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_dodgecity {
        void on_apply(game *game);
    };

    DEFINE_RULESET(dodgecity, ruleset_dodgecity)

}

#endif