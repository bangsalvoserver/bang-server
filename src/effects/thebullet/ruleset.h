#ifndef __THEBULLET_RULESET_H__
#define __THEBULLET_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_thebullet {
        void on_apply(game *game) {}
    };

    DEFINE_RULESET(thebullet, ruleset_thebullet)

}

#endif