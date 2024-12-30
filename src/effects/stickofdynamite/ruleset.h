#ifndef __STICKOFDYNAMITE_RULESET_H__
#define __STICKOFDYNAMITE_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    struct ruleset_stickofdynamite {
        void on_apply(game *game);
    };

    DEFINE_RULESET(stickofdynamite, ruleset_stickofdynamite)

}

#endif