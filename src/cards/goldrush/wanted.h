#ifndef __GOLDRUSH_WANTED_H__
#define __GOLDRUSH_WANTED_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct equip_wanted : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(wanted, equip_wanted)
}

#endif