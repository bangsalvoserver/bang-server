#ifndef __CARD_DEFS_H__
#define __CARD_DEFS_H__

#include "card_fwd.h"

namespace banggame {

    using namespace enums::flag_operators;

    DEFINE_ENUM(card_suit,
        (none)
        (hearts)
        (diamonds)
        (clubs)
        (spades)
    )

    DEFINE_ENUM(card_rank,
        (none)
        (rank_2)
        (rank_3)
        (rank_4)
        (rank_5)
        (rank_6)
        (rank_7)
        (rank_8)
        (rank_9)
        (rank_10)
        (rank_J)
        (rank_Q)
        (rank_K)
        (rank_A)
    )

    DEFINE_STRUCT(card_sign,
        (card_suit, suit)
        (card_rank, rank),

        bool is_hearts() const      { return suit == card_suit::hearts; }
        bool is_diamonds() const    { return suit == card_suit::diamonds; }
        bool is_red() const         { return is_hearts() || is_diamonds(); }
        bool is_clubs() const       { return suit == card_suit::clubs; }
        bool is_spades() const      { return suit == card_suit::spades; }
        bool is_black() const       { return is_clubs() || is_spades(); }

        bool is_two_to_nine() const {
            return enums::indexof(rank) >= enums::indexof(card_rank::rank_2)
                && enums::indexof(rank) <= enums::indexof(card_rank::rank_9);
        }

        bool is_ten_to_ace() const {
            return enums::indexof(rank) >= enums::indexof(card_rank::rank_10)
                && enums::indexof(rank) <= enums::indexof(card_rank::rank_A);
        }

        explicit operator bool () const {
            return suit != card_suit::none && rank != card_rank::none;
        }
    )

    DEFINE_ENUM_FLAGS_FWD_TYPES(expansion_type,
        (dodgecity,             ruleset_dodgecity)
        (goldrush,              ruleset_goldrush)
        (armedanddangerous,     ruleset_armedanddangerous)
        (greattrainrobbery,     ruleset_greattrainrobbery)
        (valleyofshadows,       ruleset_valleyofshadows)
        (highnoon,              ruleset_highnoon)
        (fistfulofcards,        ruleset_fistfulofcards)
        (wildwestshow,          ruleset_wildwestshow)
        (thebullet,             ruleset_thebullet)
        (canyondiablo,          ruleset_canyondiablo)
    )

    DEFINE_ENUM(card_color_type,
        (none)
        (brown)
        (blue)
        (green)
        (black)
        (orange)
        (train)
    )

    DEFINE_ENUM(player_role,
        (unknown)
        (sheriff)
        (deputy)
        (outlaw)
        (renegade)
        (deputy_3p)
        (outlaw_3p)
        (renegade_3p)
    )

    DEFINE_ENUM_TYPES(target_type,
        (none)
        (player,                serial::player)
        (conditional_player,    serial::opt_player)
        (adjacent_players,      serial::player_list)
        (player_per_cube,       serial::player_list)
        (card,                  serial::card)
        (extra_card,            serial::opt_card)
        (players)
        (cards,                 serial::card_list)
        (max_cards,             serial::card_list)
        (cards_other_players,   serial::card_list)
        (move_cube_slot,        serial::card_list)
        (select_cubes,          serial::card_list)
        (select_cubes_repeat,   serial::card_list)
        (self_cubes)
    )

    using play_card_target = enums::enum_variant<target_type>;
    using target_list = std::vector<play_card_target>;

    DEFINE_STRUCT(effect_holder,
        (target_type, target)
        (target_player_filter, player_filter)
        (target_card_filter, card_filter)
        (short, effect_value)
        (short, target_value)
        (const effect_vtable *, type)
    )

    DEFINE_STRUCT(equip_holder,
        (short, effect_value)
        (const equip_vtable *, type)
    )

    DEFINE_STRUCT(modifier_holder,
        (const modifier_vtable *, type)
    )

    DEFINE_STRUCT(mth_holder,
        (const mth_vtable *, type)
        (serial::int_list, args)
    )

    DEFINE_STRUCT(tag_holder,
        (short, tag_value)
        (tag_type, type)
    )

    using effect_list = std::vector<effect_holder>;
    using equip_list = std::vector<equip_holder>;
    using tag_list = std::vector<tag_holder>;

    struct effect_target_pair {
        const play_card_target &target;
        const effect_holder &effect;
    };
    
    using effect_target_list = std::vector<effect_target_pair>;

    DEFINE_ENUM(card_deck_type,
        (none)
        (main_deck)
        (character)
        (role)
        (goldrush)
        (highnoon)
        (fistfulofcards)
        (wildwestshow)
        (station)
        (locomotive)
        (train)
    )

    DEFINE_ENUM(pocket_type,
        (none)
        (player_hand)
        (player_table)
        (player_character)
        (player_backup)
        (main_deck)
        (discard_pile)
        (selection)
        (shop_deck)
        (shop_discard)
        (shop_selection)
        (hidden_deck)
        (scenario_deck)
        (scenario_card)
        (wws_scenario_deck)
        (wws_scenario_card)
        (button_row)
        (stations)
        (train_deck)
        (train)
    )

    DEFINE_STRUCT(card_cubes_pair,
        (serial::card, card)
        (serial::card_list, cubes)
    )

    using selected_cubes_count = std::vector<card_cubes_pair>;

    DEFINE_STRUCT(effect_context,
        (serial::player_list, selected_players)
        (serial::card_list, selected_cards)
        (selected_cubes_count, selected_cubes)
        (serial::opt_card, card_choice)
        (serial::opt_player, skipped_player)
        (serial::opt_card, repeat_card)
        (serial::opt_card, traincost)
        (serial::opt_card, playing_card)
        (int8_t, train_advance)
        (int8_t, locomotive_count)
        (int8_t, discount)
        (bool, ignore_distances)
        (bool, disable_banglimit)
        (bool, disable_bang_checks)
    )

}

#endif