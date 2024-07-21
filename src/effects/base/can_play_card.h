#ifndef __BASE_CAN_PLAY_CARD_H__
#define __BASE_CAN_PLAY_CARD_H__

#include "cards/card_effect.h"
#include "resolve.h"

namespace banggame {

    struct request_can_play_card : request_resolvable {
        using request_resolvable::request_resolvable;

        void on_update() override {
            auto_resolve();
        }

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };

    struct effect_can_play_card {
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    DEFINE_EFFECT(can_play_card, effect_can_play_card)

}

#endif