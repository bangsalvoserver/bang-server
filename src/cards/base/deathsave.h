#ifndef __BASE_DEATHSAVE_H__
#define __BASE_DEATHSAVE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_deathsave {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
    
    struct request_death : request_base, resolvable_request {
        request_death(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target, {}, 3) {}

        bool tried_save = false;

        void on_update() override;
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };
}

#endif