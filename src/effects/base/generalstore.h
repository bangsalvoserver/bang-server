#ifndef __BASE_GENERALSTORE_H__
#define __BASE_GENERALSTORE_H__

#include "cards/card_effect.h"
#include "pick.h"

namespace banggame {

    struct request_generalstore : selection_picker {
        using selection_picker::selection_picker;

        void on_update() override;

        void on_pick(card_ptr target_card) override;

        game_string status_text(player_ptr owner) const override;
    };

    struct effect_generalstore {
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
    };

    DEFINE_EFFECT(generalstore, effect_generalstore)
}

#endif