#ifndef __CANYONDIABLO_CLASH_THE_STAMPEDE__
#define __CANYONDIABLO_CLASH_THE_STAMPEDE__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_clash_the_stampede : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(clash_the_stampede, equip_clash_the_stampede)

    struct effect_clash_the_stampede {
        game_string get_error(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(clash_the_stampede, effect_clash_the_stampede)
}

#endif