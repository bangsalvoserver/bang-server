#ifndef __WILDWESTSHOW_RULESET_H__
#define __WILDWESTSHOW_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct get_count_played_cards {
            player_ptr origin;
            nullable_ref<int> value;
        };
    }

    struct ruleset_wildwestshow {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(wildwestshow, ruleset_wildwestshow)

}

#endif