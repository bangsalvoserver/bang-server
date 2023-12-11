#ifndef __GAME_EVENTS_H__
#define __GAME_EVENTS_H__

#include "cards/card_serial.h"

namespace banggame::event_type {

    DEFINE_STRUCT(on_game_setup,
        (player *, first_player)
    )
    
    DEFINE_STRUCT(apply_sign_modifier,
        (nullable_ref<card_sign>, value)
    )

    DEFINE_STRUCT(apply_maxcards_modifier,
        (player *, origin)
        (nullable_ref<int>, value)
    )

    DEFINE_STRUCT(apply_immunity_modifier,
        (card *, origin_card)
        (player *, origin)
        (const player *, target)
        (effect_flags, flags)
        (nullable_ref<std::vector<card *>>, cards)
    )

    DEFINE_STRUCT(apply_escapable_modifier,
        (card *, origin_card)
        (player *, origin)
        (const player *, target)
        (effect_flags, flags)
        (nullable_ref<int>, value)
    )
    
    DEFINE_STRUCT(count_range_mod,
        (const player *, origin)
        (range_mod_type, type)
        (nullable_ref<int>, value)
    )
    
    DEFINE_STRUCT(check_play_card,
        (player *, origin)
        (card *, origin_card)
        (const effect_context &, ctx)
        (nullable_ref<game_string>, out_error)
    )
    
    DEFINE_STRUCT(check_revivers,
        (player *, origin)
    )
    
    DEFINE_STRUCT(on_equip_card,
        (player *, origin)
        (player *, target)
        (card *, target_card)
        (const effect_context &, ctx)
    )

    DEFINE_STRUCT(on_discard_hand_card,
        (player *, origin)
        (card *, target_card)
        (bool, used)
    )

    DEFINE_STRUCT(on_turn_switch,
        (player *, origin)
    )

    DEFINE_STRUCT(pre_turn_start,
        (player *, origin)
    )

    DEFINE_STRUCT(on_turn_start,
        (player *, origin)
    )
    
    DEFINE_STRUCT(on_turn_end,
        (player *, origin)
        (bool, skipped)
    )
}

#endif