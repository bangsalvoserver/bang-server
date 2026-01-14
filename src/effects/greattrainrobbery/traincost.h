#ifndef __GREATTRAINROBBERY_TRAINCOST_H__
#define __GREATTRAINROBBERY_TRAINCOST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_traincost {
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
            return false;
        }
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(traincost, modifier_traincost)

    struct modifier_locomotive {
        bool valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };
    
    DEFINE_MODIFIER(locomotive, modifier_locomotive)

    namespace contexts {
        struct train_advance {
            struct serialize_context{};
            int value;
        };

        struct train_cost {
            card_ptr value;
        };

        struct train_card {
            struct serialize_context{};
            card_ptr value;
        };
    }

}

#endif