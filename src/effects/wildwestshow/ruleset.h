#ifndef __WILDWESTSHOW_RULESET_H__
#define __WILDWESTSHOW_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_wildwestshow {
        void on_apply(game *game) {}
    };

    DEFINE_RULESET(wildwestshow, ruleset_wildwestshow)

}

#endif