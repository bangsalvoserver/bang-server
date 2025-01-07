#ifndef __LEGENDS_KIT_CARLSON_H__
#define __LEGENDS_KIT_CARLSON_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_kit_carlson_legend : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(kit_carlson_legend, equip_kit_carlson_legend)

    struct effect_kit_carlson_legend_response {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(kit_carlson_legend_response, effect_kit_carlson_legend_response)

}

#endif