#ifndef __ARMEDANDDANGERSOUS_RULESET_H__
#define __ARMEDANDDANGERSOUS_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_armedanddangerous {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(armedanddangerous, ruleset_armedanddangerous)

    namespace event_type {
        struct get_select_cubes_prompt {
            player_ptr origin;
            const effect_context &ctx;
            nullable_ref<prompt_string> out_prompt;
        };
    }

}

#endif