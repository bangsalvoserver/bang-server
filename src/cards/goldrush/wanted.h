#ifndef __GOLDRUSH_WANTED_H__
#define __GOLDRUSH_WANTED_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_wanted : event_equip, bot_suggestion::target_enemy {
        game_string on_prompt(player *origin, card *target_card, player *target);
        void on_enable(card *target_card, player *target);
    };
}

#endif