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
        (rank_A,    "A")
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

        explicit operator bool () const {
            return suit != card_suit::none && rank != card_rank::none;
        }
    )

    DEFINE_ENUM_FLAGS_FWD_TYPES(expansion_type,
        (dodgecity)
        (goldrush,              ruleset_goldrush)
        (armedanddangerous,     ruleset_armedanddangerous)
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

    DEFINE_ENUM_FLAGS(target_player_filter,
        (any)
        (dead)
        (self)
        (notself)
        (notsheriff)
        (range_1)
        (range_2)
        (reachable)
    )

    DEFINE_ENUM_FLAGS(target_card_filter,
        (table)
        (hand)
        (blue)
        (black)
        (clubs)
        (bang)
        (bangcard)
        (missed)
        (beer)
        (bronco)
        (cube_slot)
        (can_repeat)
        (can_target_self)
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
    )

    DEFINE_ENUM_FLAGS(effect_flags,
        (is_bang)
        (play_as_bang)
        (escapable)
        (single_target)
        (multi_target)
        (auto_respond)
    )

    DEFINE_ENUM(tag_type,
        (none)
        (temp_card)
        (ghost_card)
        (confirm)
        (skip_logs)
        (hidden)
        (discard_if_two_players)
        (bangcard)
        (missedcard)
        (play_as_bang)
        (beer)
        (indians)
        (drawing)
        (weapon)
        (horse)
        (repeatable)
        (auto_confirm)
        (auto_confirm_red_ringo)
        (shopchoice)
        (last_scenario_card)
        (peyote)
        (handcuffs)
        (buy_cost)
        (max_hp)
        (bronco)
    )

    enum class effect_type : uint8_t;

    enum class equip_type : uint8_t;

    enum class mth_type : uint8_t;

    enum class modifier_type : uint8_t;

    struct effect_context {
        card *shopchoice = nullptr;
        int8_t discount = 0;
        bool ignore_distances = false;
        bool disable_banglimit = false;
        bool repeating = false;
    };

}

#endif