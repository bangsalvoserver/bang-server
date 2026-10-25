#ifndef __VLCATA_PATKA_H__
#define __VLCATA_PATKA_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct check_patka_ability {
            using result_type = bool;
            player_ptr origin;
            int ability;
        };
        struct mark_patka_used {
            player_ptr origin;
            int ability;
        };
    }

    struct equip_patka : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(patka, equip_patka)

    struct effect_patka_discard {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(patka_discard, effect_patka_discard)

}

#endif
