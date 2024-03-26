#ifndef __VALLEYOFSHADOWS_BOUNTY_H__
#define __VALLEYOFSHADOWS_BOUNTY_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct equip_bounty : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(bounty, equip_bounty)
}

#endif