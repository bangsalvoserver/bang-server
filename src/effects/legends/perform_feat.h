#ifndef __LEGENDS_PERFORM_FEAT_H__
#define __LEGENDS_PERFORM_FEAT_H__

#include "cards/card_effect.h"

namespace banggame {

    namespace event_type {
        struct get_performable_feats {
            player_ptr origin;
            nullable_ref<card_list> target_cards;
        };

        struct count_performed_feats {
            player_ptr origin;
            nullable_ref<int> num_feats;
        };

        struct check_damage_legend_kill {
            player_ptr origin;
            nullable_ref<bool> value;
        };
    }

    int get_count_performed_feats(player_ptr origin);
    std::pair<card_token_type, int> get_player_fame_tokens(const_player_ptr origin);
    void queue_request_perform_feat(card_ptr origin_card, player_ptr target);

    struct effect_perform_feat {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    
    DEFINE_EFFECT(perform_feat, effect_perform_feat)

    struct effect_damage_legend {
        bool can_play(card_ptr origin_card, player_ptr origin);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(damage_legend, effect_damage_legend)

    struct feat_equip {
        void on_disable(card_ptr origin_card, player_ptr origin);
    };

}

#endif