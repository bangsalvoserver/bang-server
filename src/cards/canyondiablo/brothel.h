#ifndef __CANYONDIABLO_BROTHEL_H__
#define __CANYONDIABLO_BROTHEL_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_brothel : event_equip, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };
}

#endif