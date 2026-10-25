#ifndef __BASE_GENERALSTORE_H__
#define __BASE_GENERALSTORE_H__

#include "cards/card_effect.h"
#include "pick.h"

namespace banggame {

    namespace event_type {
        struct count_generalstore_cards {
            nullable_ref<int> value;
        };
        struct on_generalstore_pick {
            player_ptr origin;
            card_ptr origin_card;
            player_ptr target;
            card_ptr picked_card;
        };
    }

    struct request_generalstore : selection_picker {
        using selection_picker::selection_picker;

        void on_update() override;

        void on_pick(card_ptr target_card) override;

        game_string status_text(player_ptr owner) const override;
    };

    struct effect_generalstore {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);
        
        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx);
    };

    DEFINE_EFFECT(generalstore, effect_generalstore)
}

#endif