#ifndef __BASE_CAN_PLAY_CARD_H__
#define __BASE_CAN_PLAY_CARD_H__

#include "cards/card_effect.h"
#include "resolve.h"

namespace banggame {

    struct request_can_play_card : request_dismissable {
        using request_dismissable::request_dismissable;

        void on_update() override {
            auto_resolve();
        }
        
        game_string status_text(player_ptr owner) const override;
    };

    struct effect_can_play_card {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(can_play_card, effect_can_play_card)

}

#endif