#ifndef __BASE_PROMPTS_H__
#define __BASE_PROMPTS_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct prompt_target_self {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        game_string on_prompt(card *origin_card, player *origin, card *target);
    };

    struct prompt_target_ghost {
        game_string on_prompt(card *origin_card, player *origin, player *target);
    };
}

#endif