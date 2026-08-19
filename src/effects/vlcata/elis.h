#ifndef __VLCATA_ELIS_H__
#define __VLCATA_ELIS_H__

#include "cards/card_effect.h"

namespace banggame {
    struct effect_elis {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            if (target_card->sign.is_spades()) {
                return "ERROR_CARD_IS_SPADES";
            }
            return {};
        }
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            origin->discard_card(target_card);
        }
    };
    DEFINE_EFFECT(elis, effect_elis)
}

#endif