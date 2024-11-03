#ifndef __VALLEYOFSHADOWS_EVELYN_SHEBANG_H__
#define __VALLEYOFSHADOWS_EVELYN_SHEBANG_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_evelyn_shebang {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(evelyn_shebang, effect_evelyn_shebang)
}

#endif