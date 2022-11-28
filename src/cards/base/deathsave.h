#ifndef __BASE_DEATHSAVE_H__
#define __BASE_DEATHSAVE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_deathsave {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
    
    struct request_death : request_base, resolvable_request {
        request_death(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target) {}

        bool tried_save = false;

        bool auto_resolve() override { return auto_respond(); }
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };
}

#endif