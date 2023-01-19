#ifndef __VALLEYOFSHADOWS_EVELYN_SHEBANG_H__
#define __VALLEYOFSHADOWS_EVELYN_SHEBANG_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {
    
    struct effect_evelyn_shebang : bot_suggestion::target_enemy {
        game_string verify(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif