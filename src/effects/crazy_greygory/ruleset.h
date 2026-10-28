#ifndef __CRAZY_GREYGORY_RULESET_H__
#define __CRAZY_GREYGORY_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_crazy_greygory {
        void on_apply(game_ptr game) const;
    };

    DEFINE_RULESET(crazy_greygory, ruleset_crazy_greygory)

}

#endif