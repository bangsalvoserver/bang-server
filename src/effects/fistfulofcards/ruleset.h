#ifndef __FISTFULOFCARDS_RULESET_H__
#define __FISTFULOFCARDS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_fistfulofcards {
        void on_apply(game *game);
    };

    DEFINE_RULESET(fistfulofcards, ruleset_fistfulofcards)

    namespace event_type {
        struct get_first_dead_player {
            nullable_ref<player_ptr> result;
        };
    }

}

#endif