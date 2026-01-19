#ifndef __FISTFULOFCARDS_RULESET_H__
#define __FISTFULOFCARDS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_fistfulofcards {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(fistfulofcards, ruleset_fistfulofcards)

    namespace event_type {
        struct get_first_dead_player {
            using result_type = player_ptr;
        };
    }

}

#endif