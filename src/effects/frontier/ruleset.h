#ifndef __FRONTIER_RULESET_H__
#define __FRONTIER_RULESET_H__

#include "cards/card_effect.h"

namespace banggame {

    card_token_type get_card_pardner_token(card_ptr target_card);

    player_ptr get_tracked_player(card_ptr target_card);

    void apply_pardner_token(card_ptr origin_card, player_ptr origin, player_ptr target);

    void remove_pardner_token(card_ptr origin_card, player_ptr origin);

    struct ruleset_frontier {
        void on_apply(game_ptr game);
    };

    DEFINE_RULESET(frontier, ruleset_frontier)

    struct effect_track {
        void add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx);
    };

    DEFINE_EFFECT(track, effect_track)

}

#endif