#ifndef __LEGENDS_RULESET_H__
#define __LEGENDS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_legends {
        void on_apply(game *game);
        bool is_valid_with(const expansion_set &set);
    };

    DEFINE_RULESET(legends, ruleset_legends)

}

#endif