#ifndef __HOLDERS_H__
#define __HOLDERS_H__

#include "cards/card_effect.h"

namespace banggame {

    void apply_rulesets(game *game);
    
    DEFINE_STRUCT(equip_holder,
        (short, effect_value)
        (equip_type, type),

        game_string on_prompt(card *origin_card, player *origin, player *target) const;
        void on_enable(card *target_card, player *target) const;
        void on_disable(card *target_card, player *target) const;
        bool is_nodisable() const;
    )
}

#endif