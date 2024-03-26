#ifndef __BASE_JAIL_H__
#define __BASE_JAIL_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "prompts.h"

namespace banggame {
    
    struct equip_jail : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(jail, equip_jail)
}

#endif