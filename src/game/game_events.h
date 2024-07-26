#ifndef __GAME_EVENTS_H__
#define __GAME_EVENTS_H__

#include "cards/card_fwd.h"

namespace banggame::event_type {

    struct on_game_setup {
        player *first_player;
    };
    
    struct apply_sign_modifier {
        nullable_ref<card_sign> value;
    };

    struct apply_maxcards_modifier {
        player *origin;
        nullable_ref<int> value;
    };

    struct apply_immunity_modifier {
        card *origin_card;
        player *origin;
        const player *target;
        effect_flags flags;
        nullable_ref<card_list> cards;
    };

    struct apply_escapable_modifier {
        card *origin_card;
        player *origin;
        const player *target;
        effect_flags flags;
        nullable_ref<int> value;
    };
    
    struct count_range_mod {
        const player *origin;
        range_mod_type type;
        nullable_ref<int> value;
    };
    
    struct check_play_card {
        player *origin;
        card *origin_card;
        const effect_context &ctx;
        nullable_ref<game_string> out_error;
    };
    
    struct check_revivers {
        player *origin;
    };
    
    struct on_equip_card {
        player *origin;
        player *target;
        card *target_card;
        const effect_context &ctx;
    };

    struct on_discard_hand_card {
        player *origin;
        card *target_card;
        bool used;
    };

    struct on_turn_switch {
        player *origin;
    };

    struct pre_turn_start {
        player *origin;
    };

    struct on_turn_start {
        player *origin;
    };
    
    struct on_turn_end {
        player *origin;
        bool skipped;
    };
}

#endif