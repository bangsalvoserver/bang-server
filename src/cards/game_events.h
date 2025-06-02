#ifndef __GAME_EVENTS_H__
#define __GAME_EVENTS_H__

#include "card_defs.h"

namespace banggame {
    enum class range_mod_type {
        range_mod,
        weapon_range,
        distance_mod
    };
}

namespace banggame::event_type {

    struct on_game_setup {
        player_ptr first_player;
    };

    struct on_drawn_any_card {
        nullable_ref<card_ptr> drawn_card;
    };

    struct on_play_card {
        player_ptr origin;
        card_ptr origin_card;
        card_list modifiers;
        const effect_context &ctx;
    };

    struct on_finish_tokens {
        card_ptr origin_card;
        card_ptr target_card;
        card_token_type token_type;
    };

    struct on_discard_any_card {
        player_ptr origin;
        card_ptr target_card;
    };

    struct apply_maxcards_modifier {
        const_player_ptr origin;
        nullable_ref<int> value;
    };

    struct apply_immunity_modifier {
        card_ptr origin_card;
        player_ptr origin;
        const_player_ptr target;
        effect_flags flags;
        nullable_ref<card_list> cards;
    };
    
    struct count_range_mod {
        const_player_ptr origin;
        range_mod_type type;
        nullable_ref<int> value;
    };

    struct count_initial_cards {
        const_player_ptr origin;
        nullable_ref<int> value;
    };
    
    struct check_play_card {
        player_ptr origin;
        card_ptr origin_card;
        const effect_context &ctx;
        nullable_ref<game_string> out_error;
    };
    
    struct check_revivers {
        player_ptr origin;
    };
    
    struct on_equip_card {
        player_ptr origin;
        player_ptr target;
        card_ptr target_card;
        const effect_context &ctx;
    };

    struct on_discard_hand_card {
        player_ptr origin;
        card_ptr target_card;
        bool used;
    };

    struct on_turn_switch {
        player_ptr origin;
    };

    struct pre_turn_start {
        player_ptr origin;
    };

    struct on_turn_start {
        player_ptr origin;
    };
    
    struct on_turn_end {
        player_ptr origin;
        bool skipped;
    };
}

#endif