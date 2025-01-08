#ifndef __LEGENDS_RULESET_H__
#define __LEGENDS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    card_ptr draw_next_feat(player_ptr origin);

    struct ruleset_legends {
        void on_apply(game_ptr game);
        bool is_valid_with(const expansion_set &set);
    };

    DEFINE_RULESET(legends, ruleset_legends)

}

#endif