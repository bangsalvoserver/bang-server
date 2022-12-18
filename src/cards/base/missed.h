#ifndef __BASE_MISSED_H__
#define __BASE_MISSED_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_missed {
        bool can_respond(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
}

#endif