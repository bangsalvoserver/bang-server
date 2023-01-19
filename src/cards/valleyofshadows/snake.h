#ifndef __VALLEYOFSHADOWS_SNAKE_H__
#define __VALLEYOFSHADOWS_SNAKE_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_snake : event_equip, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };
}

#endif