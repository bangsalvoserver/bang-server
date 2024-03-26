#ifndef __VALLEYOFSHADOWS_SNAKE_H__
#define __VALLEYOFSHADOWS_SNAKE_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct equip_snake : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(snake, equip_snake)
}

#endif