#ifndef __BASE_GENERALSTORE_H__
#define __BASE_GENERALSTORE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct request_generalstore : selection_picker {
        request_generalstore(card *origin_card, player *origin, player *target)
            : selection_picker(origin_card, origin, target) {}

        void on_update() override;

        void on_pick(card *target_card) override;

        game_string status_text(player *owner) const override;
    };

    struct effect_generalstore {
        void on_play(card *origin_card, player *origin, const effect_context &ctx);
        void on_play(card *origin_card, player *origin, player *target);
    };

    DEFINE_EFFECT(generalstore, effect_generalstore)
}

#endif