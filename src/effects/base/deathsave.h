#ifndef __BASE_DEATHSAVE_H__
#define __BASE_DEATHSAVE_H__

#include "cards/card_effect.h"
#include "resolve.h"

namespace banggame {

    namespace event_type {
        DEFINE_STRUCT(on_player_death_resolve,
            (player *, target)
            (bool, tried_save)
        )
        
        DEFINE_STRUCT(on_player_death,
            (player *, origin)
            (player *, target)
        )
    }

    struct effect_deathsave {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(deathsave, effect_deathsave)
    
    struct request_death : request_resolvable {
        request_death(card *origin_card, player *origin, player *target)
            : request_resolvable(origin_card, origin, target, {}, 50) {}

        bool tried_save = false;

        void on_update() override;
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };
}

#endif