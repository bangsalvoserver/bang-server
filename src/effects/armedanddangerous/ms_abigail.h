#ifndef __ARMEDANDDANGEROUS_MS_ABIGAIL__
#define __ARMEDANDDANGEROUS_MS_ABIGAIL__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_ms_abigail : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EQUIP(ms_abigail, equip_ms_abigail)

    struct effect_ms_abigail {
        static bool can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags);
        
        bool can_play(card_ptr origin_card, player_ptr target) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr target) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(ms_abigail, effect_ms_abigail)
}

#endif