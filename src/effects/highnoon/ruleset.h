#ifndef __HIGHNOON_RULESET_H__
#define __HIGHNOON_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    void draw_scenario_card(game *game);

    struct ruleset_highnoon {
        void on_apply(game *game);
    };

    DEFINE_RULESET(highnoon, ruleset_highnoon)

}

#endif