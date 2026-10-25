#ifndef __VLCATA_RULESET_H__
#define __VLCATA_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_vlcata {
        bool is_valid_with(const expansion_set &set);
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(vlcata, ruleset_vlcata)

}

#endif