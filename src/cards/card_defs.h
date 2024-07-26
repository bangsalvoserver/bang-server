#ifndef __CARD_DEFS_H__
#define __CARD_DEFS_H__

#include "game_string.h"

#include "utils/tagged_variant.h"
#include "utils/enum_bitset.h"

namespace banggame {

    enum class card_suit {
        none,
        hearts,
        diamonds,
        clubs,
        spades,
    };

    enum class card_rank {
        none,
        rank_2,
        rank_3,
        rank_4,
        rank_5,
        rank_6,
        rank_7,
        rank_8,
        rank_9,
        rank_10,
        rank_J,
        rank_Q,
        rank_K,
        rank_A,
    };

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

    enum class expansion_type {
        dodgecity,
        goldrush,
        armedanddangerous,
        greattrainrobbery,
        valleyofshadows,
        highnoon,
        fistfulofcards,
        wildwestshow,
        thebullet,
        canyondiablo,
    };

    enum class card_color_type {
        none,
        brown,
        blue,
        green,
        black,
        orange,
        train,
    };

    enum class player_role {
        unknown,
        sheriff,
        deputy,
        outlaw,
        renegade,
        deputy_3p,
        outlaw_3p,
        renegade_3p,
    };

    using play_card_target = utils::tagged_variant<
        utils::tag<"none">,
        utils::tag<"player", player_ptr>,
        utils::tag<"conditional_player", nullable_player>,
        utils::tag<"adjacent_players", player_list>,
        utils::tag<"player_per_cube", player_list>,
        utils::tag<"card", card_ptr>,
        utils::tag<"extra_card", nullable_card>,
        utils::tag<"players">,
        utils::tag<"cards", card_list>,
        utils::tag<"max_cards", card_list>,
        utils::tag<"card_per_player", card_list>,
        utils::tag<"move_cube_slot", card_list>,
        utils::tag<"select_cubes", card_list>,
        utils::tag<"select_cubes_optional", card_list>,
        utils::tag<"select_cubes_repeat", card_list>,
        utils::tag<"select_cubes_players", card_list>,
        utils::tag<"self_cubes">
    >;

    using target_type = utils::tagged_variant_index<play_card_target>;
    #define TARGET_TYPE(name) target_type{utils::tag<#name>{}}

    using target_list = std::vector<play_card_target>;

    struct effect_holder {
        target_type target;
        enums::bitset<target_player_filter> player_filter;
        enums::bitset<target_card_filter> card_filter;
        short effect_value;
        short target_value;
        const effect_vtable *type;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const;

        game_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) const;

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) const;
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) const;

        void on_play(card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) const;
    };

    struct equip_holder {
        short effect_value;
        const equip_vtable *type;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const;
        void on_enable(card_ptr target_card, player_ptr target) const;
        void on_disable(card_ptr target_card, player_ptr target) const;
        bool is_nodisable() const;
    };

    struct modifier_holder {
        const modifier_vtable *type;

        explicit operator bool () const {
            return type != nullptr;
        }

        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) const;
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr playing_card, const effect_context &ctx) const;
    };

    struct mth_holder {
        const mth_vtable *type;
        small_int_set args;

        explicit operator bool () const {
            return type != nullptr;
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
        game_string on_prompt(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
        void on_play(card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) const;
    };

    using effect_list = std::vector<effect_holder>;
    using equip_list = std::vector<equip_holder>;
    using tag_map = std::unordered_map<tag_type, short>;

    enum class card_deck_type {
        none,
        main_deck,
        character,
        role,
        goldrush,
        highnoon,
        fistfulofcards,
        wildwestshow,
        station,
        locomotive,
        train,
    };

    enum class pocket_type {
        none,
        player_hand,
        player_table,
        player_character,
        player_backup,
        main_deck,
        discard_pile,
        selection,
        shop_deck,
        shop_discard,
        shop_selection,
        hidden_deck,
        scenario_deck,
        scenario_card,
        wws_scenario_deck,
        wws_scenario_card,
        button_row,
        stations,
        train_deck,
        train,
    };

    class selected_cubes_count {
    private:
        std::unordered_multimap<const_card_ptr, card_ptr> m_value;

    public:
        void insert(const_card_ptr origin_card, card_ptr cube) {
            m_value.emplace(origin_card, cube);
        }

        auto operator[](const_card_ptr origin_card) const {
            auto [low, high] = m_value.equal_range(origin_card);
            return rn::subrange(low, high) | rv::values;
        }

        int count(const_card_ptr origin_card) const {
            return static_cast<int>(rn::distance((*this)[origin_card]));
        }

        auto all() const {
            return m_value | rv::values;
        }
    };

    struct effect_context_base {
        nullable_card playing_card;
        nullable_card repeat_card;
        nullable_card card_choice;
        int8_t train_advance;
        bool ignore_distances;
    };

    struct effect_context : effect_context_base {
        player_list selected_players;
        card_list selected_cards;
        selected_cubes_count selected_cubes;
        nullable_player skipped_player;
        nullable_card traincost;
        int8_t locomotive_count;
        int8_t discount;
        bool disable_banglimit;
        bool disable_bang_checks;
    };

}

#endif