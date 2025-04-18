#ifndef __BASE_JAIL_H__
#define __BASE_JAIL_H__

#include "steal_destroy.h"

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_jail : event_equip {
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(jail, equip_jail)

    struct effect_escape_jail : effect_discard {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };

    DEFINE_EFFECT(escape_jail, effect_escape_jail)
}

#endif