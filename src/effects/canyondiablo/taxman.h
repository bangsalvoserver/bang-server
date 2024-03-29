#ifndef __CANYONDIABLO_TAXMAN_H__
#define __CANYONDIABLO_TAXMAN_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "effects/base/prompts.h"

namespace banggame {

    struct equip_taxman : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(taxman, equip_taxman)
}

#endif