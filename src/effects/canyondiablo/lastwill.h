#ifndef __CANYONDIABLO_LASTWILL_H__
#define __CANYONDIABLO_LASTWILL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_lastwill : event_equip {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        int evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EQUIP(lastwill, equip_lastwill)

    struct effect_lastwill {
        bool can_play(card_ptr origin_card, player_ptr origin) const;

        int evaluate(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(lastwill, effect_lastwill)
}

#endif
