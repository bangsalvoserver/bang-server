#ifndef __VLCATA_ANEZKA_H__
#define __VLCATA_ANEZKA_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct check_anezka_can_heal {
            using result_type = bool;
            player_ptr origin;
            player_ptr target;
        };
        struct mark_anezka_healed {
            player_ptr origin;
            player_ptr target;
        };
    }

    struct equip_anezka : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(anezka, equip_anezka)

    struct effect_anezka_heal {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };
    DEFINE_EFFECT(anezka_heal, effect_anezka_heal)

}

#endif
