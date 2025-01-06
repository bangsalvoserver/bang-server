#ifndef __LEGENDS_PERFORM_FEAT_H__
#define __LEGENDS_PERFORM_FEAT_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct count_performed_feats {
            player_ptr origin;
            nullable_ref<int> num_feats;
        };

        struct check_claim_feat_kill {
            player_ptr origin;
            nullable_ref<bool> value;
        };
    }

    int get_count_performed_feats(player_ptr origin);
    bool is_legend(const_player_ptr origin);
    std::pair<card_token_type, int> get_card_fame_token_type(const_card_ptr origin_card);

    struct effect_perform_feat {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    
    DEFINE_EFFECT(perform_feat, effect_perform_feat)

}

#endif