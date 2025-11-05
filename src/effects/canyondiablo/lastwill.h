#ifndef __CANYONDIABLO_LASTWILL_H__
#define __CANYONDIABLO_LASTWILL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lastwill : event_equip {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(lastwill, equip_lastwill)

    struct effect_lastwill {
        bool can_play(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
    };

    DEFINE_EFFECT(lastwill, effect_lastwill)
}

#endif