#ifndef __ARMEDANDDANGERSOUS_RULESET_H__
#define __ARMEDANDDANGERSOUS_RULESET_H__

#include "cards/card_fwd.h"

namespace banggame {

    namespace event_type {
        DEFINE_STRUCT(on_discard_orange_card,
            (player *, target)
            (card *, target_card)
        )
    }

    struct ruleset_armedanddangerous {
        void on_apply(game *game);
    };

}

#endif