#ifndef __GREATTRAINROBBERY_LUMBER_FLATCAR_H__
#define __GREATTRAINROBBERY_LUMBER_FLATCAR_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct equip_lumber_flatcar : event_equip, prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
    };
}

#endif