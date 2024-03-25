#ifndef __GREATTRAINROBBERY_KNIFE_REVOLVER_H__
#define __GREATTRAINROBBERY_KNIFE_REVOLVER_H__

#include "cards/card_effect.h"

#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_knife_revolver : bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(knife_revolver, effect_knife_revolver)
}

#endif