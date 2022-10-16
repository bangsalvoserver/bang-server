#ifndef __BASE_SCOPE_H__
#define __BASE_SCOPE_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_scope {
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };
}

#endif