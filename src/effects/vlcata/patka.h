#ifndef __VLCATA_PATKA_H__
#define __VLCATA_PATKA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_patka : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    struct effect_patka_mass {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };

    struct effect_patka_discard_n {
        void on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards);
    };

}

#endif