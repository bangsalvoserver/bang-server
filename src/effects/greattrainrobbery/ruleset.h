#ifndef __GREATTRAINROBBERY_RULESET_H__
#define __GREATTRAINROBBERY_RULESET_H__

#include <memory>

#include "cards/card_effect.h"

namespace banggame {

    struct locomotive_context {
        int8_t locomotive_count;
        player_ptr skipped_player;
    };

    using shared_locomotive_context = std::shared_ptr<locomotive_context>;

    namespace event_type {
        struct count_train_equips {
            player_ptr origin;
            nullable_ref<int> num_cards;
            nullable_ref<int> num_advance;
        };
        
        struct on_train_advance {
            player_ptr origin;
            shared_locomotive_context ctx;
        };

        struct on_locomotive_effect {
            player_ptr origin;
            shared_locomotive_context ctx;
        };

        struct get_locomotive_prompt {
            using result_type = prompt_string;
            player_ptr origin;
            int locomotive_count;
        };
    }
    
    struct ruleset_greattrainrobbery {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(greattrainrobbery, ruleset_greattrainrobbery)
}

#endif