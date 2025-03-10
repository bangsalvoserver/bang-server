#ifndef __BASE_DEATH_H__
#define __BASE_DEATH_H__

#include "cards/card_effect.h"
#include "resolve.h"

namespace banggame {

    enum class death_type {
        death,
        ghost_discard,
        ghosttown_turn_end,
        shadow_turn_end
    };
    
    void handle_player_death(player_ptr killer, player_ptr target, death_type type);

    namespace event_type {
        struct on_player_death {
            player_ptr target;
            bool tried_save;
        };
        
        struct on_player_eliminated {
            player_ptr origin;
            player_ptr target;
            death_type type;
        };

        struct check_remove_player {
            nullable_ref<bool> value;
        };
    }

    struct effect_deathsave {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(deathsave, effect_deathsave)
    
    struct request_death : request_resolvable {
        request_death(card_ptr origin_card, player_ptr origin, player_ptr target)
            : request_resolvable(origin_card, origin, target, {}, 50) {}

        bool tried_save = false;

        void on_update() override;
        void on_resolve() override;
        prompt_string resolve_prompt() const override;
        game_string status_text(player_ptr owner) const override;
    };
}

#endif