#ifndef __CARD_ENUMS_H__
#define __CARD_ENUMS_H__

#include "utils/enum_variant.h"
#include "utils/reflector.h"

#include "card_serial.h"

namespace banggame {

    using namespace enums::flag_operators;

    DEFINE_ENUM_DATA(card_suit,
        (none,      u8"")
        (hearts,    u8"\u2665")
        (diamonds,  u8"\u2666")
        (clubs,     u8"\u2663")
        (spades,    u8"\u2660")
    )

    DEFINE_ENUM_DATA(card_rank,
        (none,      "")
        (rank_2,    "2")
        (rank_3,    "3")
        (rank_4,    "4")
        (rank_5,    "5")
        (rank_6,    "6")
        (rank_7,    "7")
        (rank_8,    "8")
        (rank_9,    "9")
        (rank_10,   "10")
        (rank_J,    "J")
        (rank_Q,    "Q")
        (rank_K,    "K")
        (rank_A,    "A")
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
        (dodgecity)
        (goldrush,              ruleset_goldrush)
        (armedanddangerous,     ruleset_armedanddangerous)
        (greattrainrobbery,     ruleset_greattrainrobbery)
        (valleyofshadows,       ruleset_valleyofshadows)
        (highnoon)
        (fistfulofcards)
        (wildwestshow)
        (thebullet)
        (canyondiablo,          ruleset_canyondiablo)
    )

    constexpr auto unofficial_expansions = expansion_type::canyondiablo;

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
        (card,                  serial::card)
        (extra_card,            serial::opt_card)
        (players)
        (cards,                 serial::card_list)
        (cards_other_players,   serial::card_list)
        (select_cubes,          serial::card_list)
        (self_cubes)
    )

    using play_card_target = enums::enum_variant<target_type>;
    using target_list = std::vector<play_card_target>;

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
        (stations_deck)
        (stations)
        (train_deck)
        (train)
    )

    enum class effect_flags;
    enum class game_flags;
    enum class player_flags;

    enum class effect_type;
    enum class equip_type;
    enum class mth_type;
    enum class modifier_type;

    enum class target_player_filter;
    enum class target_card_filter;
    enum class tag_type;

}

#endif