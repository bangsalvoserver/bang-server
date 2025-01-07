#ifndef __LEGENDS_JOURDONNAIS__
#define __LEGENDS_JOURDONNAIS__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_jourdonnais_legend : event_equip {
        void on_enable(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EQUIP(jourdonnais_legend, equip_jourdonnais_legend)

    struct effect_jourdonnais_legend {
        static bool can_escape(player_ptr origin, card_ptr origin_card, effect_flags flags);
        
        bool can_play(card_ptr origin_card, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(jourdonnais_legend, effect_jourdonnais_legend)
}

#endif