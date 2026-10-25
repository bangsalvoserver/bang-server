#ifndef __LEGENDS_SID_KETCHUM_H__
#define __LEGENDS_SID_KETCHUM_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_sid_ketchum_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target) const;
    };

    DEFINE_EQUIP(sid_ketchum_legend, equip_sid_ketchum_legend)

    struct effect_sid_ketchum_legend_free_bang {
        bool can_play(card_ptr origin_card, player_ptr origin) const;
        void on_play(card_ptr origin_card, player_ptr origin) const;
    };

    DEFINE_EFFECT(sid_ketchum_legend_free_bang, effect_sid_ketchum_legend_free_bang)

}

#endif