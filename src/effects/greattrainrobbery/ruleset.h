#ifndef __GREATTRAINROBBERY_RULESET_H__
#define __GREATTRAINROBBERY_RULESET_H__

#include <memory>

#include "cards/card_effect.h"

namespace banggame {

    struct effect_context;
    using shared_effect_context = std::shared_ptr<effect_context>;

    namespace event_type {
        struct count_train_equips {
            player *origin;
            nullable_ref<int> num_cards;
            nullable_ref<int> num_advance;
        };
        
        struct on_train_advance {
            player *origin;
            shared_effect_context ctx;
        };

        struct on_locomotive_effect {
            player *origin;
            shared_effect_context ctx;
        };
    }
    
    struct ruleset_greattrainrobbery {
        void on_apply(game *game);
    };

    DEFINE_RULESET(greattrainrobbery, ruleset_greattrainrobbery)
}

#endif