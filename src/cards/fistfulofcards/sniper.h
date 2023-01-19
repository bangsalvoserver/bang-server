#ifndef __FISTFULOFCARDS_SNIPER_H__
#define __FISTFULOFCARDS_SNIPER_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_sniper : bot_suggestion::target_enemy {
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif