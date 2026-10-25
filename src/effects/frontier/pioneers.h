#ifndef __FRONTIER_PIONEERS_H__
#define __FRONTIER_PIONEERS_H__

#include "cards/card_effect.h"

#include "effects/base/equip.h"

namespace banggame {

    struct effect_equip_on_next {
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
    };

    DEFINE_EFFECT(equip_on_next, effect_equip_on_next)

    struct equip_pioneers {
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(pioneers, equip_pioneers)
}

#endif