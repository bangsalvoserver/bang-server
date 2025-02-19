#ifndef __HIGHNOON_RULESET_H__
#define __HIGHNOON_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    void draw_scenario_card(game_ptr game);

    struct ruleset_highnoon {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(highnoon, ruleset_highnoon)

}

#endif