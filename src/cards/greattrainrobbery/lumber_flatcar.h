#ifndef __GREATTRAINROBBERY_LUMBER_FLATCAR_H__
#define __GREATTRAINROBBERY_LUMBER_FLATCAR_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"
#include "cards/base/prompts.h"

namespace banggame {

    struct equip_lumber_flatcar : prompt_target_self, bot_suggestion::target_enemy {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    DEFINE_EQUIP(lumber_flatcar, equip_lumber_flatcar)
}

#endif