#ifndef __GREATTRAINROBBERY_TRAIN_ROBBERY_H__
#define __GREATTRAINROBBERY_TRAIN_ROBBERY_H__

#include "cards/card_effect.h"

#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_train_robbery : bot_suggestion::target_enemy {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags);
    };
}

#endif