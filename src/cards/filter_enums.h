#ifndef __CARDS_FILTER_ENUMS_H__
#define __CARDS_FILTER_ENUMS_H__

#include "utils/enums.h"

namespace banggame {

    DEFINE_ENUM_FLAGS(target_player_filter,
        (any)
        (dead)
        (alive)
        (self)
        (notself)
        (notsheriff)
        (notorigin)
        (range_1)
        (range_2)
        (reachable)
        (target_set)
        (not_empty_hand)
        (not_empty_table)
        (not_empty_cubes)
    )

    DEFINE_ENUM_FLAGS(target_card_filter,
        (pick_card)
        (selection)
        (table)
        (hand)
        (blue)
        (black)
        (train)
        (nottrain)
        (blue_or_train)
        (hearts)
        (diamonds)
        (clubs)
        (spades)
        (origin_card_suit)
        (two_to_nine)
        (ten_to_ace)
        (bang)
        (bangcard)
        (not_bangcard)
        (missed)
        (missedcard)
        (not_missedcard)
        (beer)
        (bronco)
        (cube_slot)
        (can_target_self)
        (catbalou_panic)
    )
    
    DEFINE_ENUM(tag_type,
        (none)
        (ghost_card)
        (confirm)
        (pass_turn)
        (pick)
        (skip_logs)
        (no_auto_discard)
        (bangcard)
        (missed)
        (missedcard)
        (play_as_bang)
        (banglimit)
        (beer)
        (indians)
        (panic)
        (cat_balou)
        (drawing)
        (weapon)
        (horse)
        (auto_select)
        (card_choice)
        (last_scenario_card)
        (peyote)
        (handcuffs)
        (buy_cost)
        (max_hp)
        (initial_cards)
        (bronco)
    )
}

#endif