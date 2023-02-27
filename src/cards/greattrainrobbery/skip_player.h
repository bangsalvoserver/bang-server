#ifndef __GREATTRAINROBBERY_SKIP_PLAYER_H__
#define __GREATTRAINROBBERY_SKIP_PLAYER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_skip_player {
        game_string get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx);
        void add_context(card *origin_card, player *origin, player *target, effect_context &ctx);
    };
}

#endif