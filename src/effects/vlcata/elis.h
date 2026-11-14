#ifndef __VLCATA_ELIS_H__
#define __VLCATA_ELIS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_elis {
        bool can_target(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };

    DEFINE_EFFECT(elis, effect_elis)
}

#endif