#ifndef __ARMEDANDDANGERSOUS_RULESET_H__
#define __ARMEDANDDANGERSOUS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_armedanddangerous {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(armedanddangerous, ruleset_armedanddangerous)

}

#endif