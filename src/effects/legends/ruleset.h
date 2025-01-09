#ifndef __LEGENDS_RULESET_H__
#define __LEGENDS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    card_ptr draw_next_feat(player_ptr origin);

    struct ruleset_legends {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(legends, ruleset_legends)

}

#endif