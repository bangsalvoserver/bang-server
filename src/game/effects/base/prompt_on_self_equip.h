#ifndef __BASE_PROMPT_ON_SELF_EQUIP_H__
#define __BASE_PROMPT_ON_SELF_EQUIP_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_prompt_on_self_equip {
        game_string on_prompt(player *origin, card *target_card, player *target);
    };
}

#endif