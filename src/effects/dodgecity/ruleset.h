#ifndef __DODGECITY_RULESET_H__
#define __DODGECITY_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct check_equipped_green_card {
            player_ptr origin;
            card_ptr origin_card;
            nullable_ref<bool> value;
        };
    };

    struct ruleset_dodgecity {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(dodgecity, ruleset_dodgecity)

}

#endif