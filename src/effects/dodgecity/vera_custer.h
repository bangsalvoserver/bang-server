#ifndef __DODGECITY_VERA_CUSTER_H__
#define __DODGECITY_VERA_CUSTER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_vera_custer : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(vera_custer, equip_vera_custer)

    struct effect_vera_custer {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(vera_custer, effect_vera_custer)
}

#endif