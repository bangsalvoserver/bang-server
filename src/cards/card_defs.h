#ifndef __CARD_DEFS_H__
#define __CARD_DEFS_H__

#include "game_string.h"

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

    struct card_sign {
        card_suit suit;
        card_rank rank;

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
    };

    DEFINE_ENUM_FLAGS(expansion_type,
        (dodgecity)
        (goldrush)
        (armedanddangerous)
        (greattrainrobbery)
        (valleyofshadows)
        (highnoon)
        (fistfulofcards)
        (wildwestshow)
        (thebullet)
        (canyondiablo)
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
        (card_per_player,       serial::card_list)
        (move_cube_slot,        serial::card_list)
        (select_cubes,          serial::card_list)
        (select_cubes_optional, serial::card_list)
        (select_cubes_repeat,   serial::card_list)
        (select_cubes_players,  serial::card_list)
        (self_cubes)
    )

    using play_card_target = enums::enum_variant<target_type>;
    using target_list = std::vector<play_card_target>;

    struct effect_holder {
        target_type target;
        target_player_filter player_filter;
        target_card_filter card_filter;
        short effect_value;
        short target_value;
        const effect_vtable *type;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string get_error(card *origin_card, player *origin, const effect_context &ctx) const;
        game_string get_error(card *origin_card, player *origin, player *target, const effect_context &ctx) const;
        game_string get_error(card *origin_card, player *origin, card *target, const effect_context &ctx) const;

        game_string on_prompt(card *origin_card, player *origin, const effect_context &ctx) const;
        game_string on_prompt(card *origin_card, player *origin, player *target, const effect_context &ctx) const;
        game_string on_prompt(card *origin_card, player *origin, card *target, const effect_context &ctx) const;

        void add_context(card *origin_card, player *origin, effect_context &ctx) const;
        void add_context(card *origin_card, player *origin, player *target, effect_context &ctx) const;
        void add_context(card *origin_card, player *origin, card *target, effect_context &ctx) const;

        void on_play(card *origin_card, player *origin, effect_flags flags, const effect_context &ctx) const;
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags, const effect_context &ctx) const;
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags, const effect_context &ctx) const;
    };

    struct equip_holder {
        short effect_value;
        const equip_vtable *type;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string on_prompt(card *origin_card, player *origin, player *target) const;
        void on_enable(card *target_card, player *target) const;
        void on_disable(card *target_card, player *target) const;
        bool is_nodisable() const;
    };

    struct modifier_holder {
        const modifier_vtable *type;

        explicit operator bool () const {
            return type != nullptr;
        }

        void add_context(card *origin_card, player *origin, effect_context &ctx) const;
        game_string get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx) const;
        game_string on_prompt(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) const;
    };

    struct mth_holder {
        const mth_vtable *type;
        serial::int_list args;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string get_error(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx) const;
        game_string on_prompt(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx) const;
        void on_play(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx) const;
    };

    using effect_list = std::vector<effect_holder>;
    using equip_list = std::vector<equip_holder>;
    using tag_map = std::unordered_map<tag_type, short>;

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

    class selected_cubes_count {
    private:
        std::unordered_multimap<const card *, card *> m_value;

    public:
        void insert(const card *origin_card, card *cube) {
            m_value.emplace(origin_card, cube);
        }

        auto operator[](const card *origin_card) const {
            auto [low, high] = m_value.equal_range(origin_card);
            return rn::subrange(low, high) | rv::values;
        }

        int count(const card *origin_card) const {
            return static_cast<int>(rn::distance((*this)[origin_card]));
        }

        auto all() const {
            return m_value | rv::values;
        }
    };

    struct effect_context {
        card *playing_card;
        std::vector<player *> selected_players;
        std::vector<card *> selected_cards;
        selected_cubes_count selected_cubes;
        card *card_choice;
        player *skipped_player;
        card *repeat_card;
        card *traincost;
        int8_t train_advance;
        int8_t locomotive_count;
        int8_t discount;
        bool ignore_distances;
        bool disable_banglimit;
        bool disable_bang_checks;
    };

}

#endif