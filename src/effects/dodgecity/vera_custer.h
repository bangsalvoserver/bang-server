#ifndef __DODGECITY_VERA_CUSTER_H__
#define __DODGECITY_VERA_CUSTER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vera_custer : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(vera_custer, equip_vera_custer)

    struct effect_vera_custer {
        game_string get_error(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(vera_custer, effect_vera_custer)
}

#endif